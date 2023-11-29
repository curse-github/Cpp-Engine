#include "Main.h"

#pragma region FpsTracker
FpsTracker::FpsTracker(Engine* _engine) : Object(_engine) {
	if(!initialized) return;
	engine->sub_loop(this);
}
void FpsTracker::on_loop(double delta) {
	if(engine->ended||!initialized) return;
	for(unsigned int i=1; i<60; i++) { lastFrames[i-1]=lastFrames[i]; }// move values back
	lastFrames[59]=delta;// put delta at the end
	double sum=0.0f;
	for(unsigned int i=0; i<60; i++) { sum+=lastFrames[i]; }// sum values
	avgFps=(int)(60.0f/sum+0.5);
	frameTime=(float)sum;//1000.0f*1000.0f;

	highFps=0;
	lowFps=100000;
	for(unsigned int i=0; i<60; i++) {
		if((1/lastFrames[i])>highFps) highFps=(int)((1/lastFrames[i])+0.5);
		if((1/lastFrames[i])<lowFps) lowFps=(int)((1/lastFrames[i])+0.5);
	}
}
int FpsTracker::getAvgFps() {
	if(engine->ended||!initialized) return 0;
	return avgFps;
}
int FpsTracker::getHighFps() {
	if(engine->ended||!initialized) return 0;
	return highFps;
}
int FpsTracker::getLowFps() {
	if(engine->ended||!initialized) return 0;
	return lowFps;
}
float FpsTracker::getFrameTime() {
	if(engine->ended||!initialized) return 0;
	return frameTime;
}
#pragma endregion// FpsTracker

#pragma region PlayerController
PlayerController::PlayerController(Engine* _engine, SpriteRenderer* _playerRenderer, BoxCollider* _playerCollider, SpriteRenderer* _flashlightRenderer, SpriteRenderer* _playerIconRenderer, OrthoCam* _sceneCam)
	: Object(_engine), playerRenderer(_playerRenderer), playerCollider(_playerCollider), flashlightRenderer(_flashlightRenderer), playerIconRenderer(_playerIconRenderer), sceneCam(_sceneCam) {
	if(!initialized) return;
	engine->sub_key(this);
	engine->sub_loop(this);
	for(unsigned int i=0; i<instanceColliders.size(); i++) {
		CollitionData collition=instanceColliders[i]->checkCollision(playerCollider);
		playerCollider->pos+=collition.normal*collition.dist;
	}
	Vector2 pos=playerCollider->pos;
	playerRenderer->position=pos;
	flashlightRenderer->position=pos;
	sceneCam->position=pos;
	playerIconRenderer->position=gridToMinimap(worldToGrid(pos));
	sceneCam->update();
	sceneCam->use();
}
void PlayerController::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(engine->ended||!initialized) return;
	if(key==GLFW_KEY_W) inputs[0]=action;
	else if(key==GLFW_KEY_A) inputs[1]=action;
	else if(key==GLFW_KEY_S) inputs[2]=action;
	else if(key==GLFW_KEY_D) inputs[3]=action;
	else if(key==GLFW_KEY_LEFT_SHIFT) inputs[4]=action;
}
void PlayerController::on_loop(double delta) {
	if(engine->ended||!initialized) return;
	Vector2 inputVec=Vector2(
		(float)(inputs[3]>=GLFW_PRESS)-(float)(inputs[1]>=GLFW_PRESS),
		(float)(inputs[0]>=GLFW_PRESS)-(float)(inputs[2]>=GLFW_PRESS)
	).normalized();
	if(inputVec.x==0&&inputVec.y==0) return;
	playerCollider->pos+=inputVec*((float)delta)*((inputs[4]>=GLFW_PRESS) ? playerSprintSpeed : playerSpeed)*mapScale*(1+spacing);
	for(unsigned int i=0; i<instanceColliders.size(); i++) {
		CollitionData collition=instanceColliders[i]->checkCollision(playerCollider);
		playerCollider->pos+=collition.normal*collition.dist;
	}
	Vector2 pos=playerCollider->pos;
	playerRenderer->position=pos;
	flashlightRenderer->position=pos;
	sceneCam->position=pos;
	playerIconRenderer->position=gridToMinimap(worldToGrid(pos));
	sceneCam->update();
	sceneCam->use();
}
void PlayerController::setPos(Vector2 pos) {
	playerCollider->pos=pos;
	for(unsigned int i=0; i<instanceColliders.size(); i++) {
		CollitionData collition=instanceColliders[i]->checkCollision(playerCollider);
		playerCollider->pos+=collition.normal*collition.dist;
	}
	Vector2 newPos=playerCollider->pos;
	playerRenderer->position=newPos;
	flashlightRenderer->position=newPos;
	sceneCam->position=newPos;
	playerIconRenderer->position=gridToMinimap(worldToGrid(newPos));
	sceneCam->update();
	sceneCam->use();
}
Vector2 PlayerController::getPos() {
	return playerCollider->pos;
}
#pragma endregion// PlayerController

#pragma region BoxCollider
BoxCollider::BoxCollider(Engine* _engine, Vector2 _pos, Vector2 _size, Shader* _debugLineShader) : Object(_engine), pos(_pos), size(_size), debugLineShader(_debugLineShader) {
	std::vector<Vector2> positions;
	positions.push_back(Vector2(-size.x/2, size.y/2));
	positions.push_back(Vector2(size.x/2, size.y/2));
	positions.push_back(Vector2(size.x/2, -size.y/2));
	positions.push_back(Vector2(-size.x/2, -size.y/2));
	debugRenderer=new LineRenderer(engine, debugLineShader, positions, 2.0f, _pos, true);
	boundingRadius=sqrt(size.x*size.x+size.y*size.y);
	engine->sub_delete(this);
}
void BoxCollider::drawOutline() {
	debugRenderer->position=pos;
	if(ColliderDebug) debugRenderer->draw();
}
void BoxCollider::on_delete() {
	delete debugRenderer;
}
CollitionData BoxCollider::checkCollision(BoxCollider* other) {
	if((pos-other->pos).length()-boundingRadius-other->boundingRadius>=0) CollitionData(Vector2(0.0f, 0.0f), 0.0f);
	// collision x-axis?
	float collisionX1=((pos.x+size.x/2)-(other->pos.x-other->size.x/2));
	float collisionX2=((other->pos.x+other->size.x/2)-(pos.x-size.x/2));
	// collision y-axis?
	float collisionY1=((pos.y+size.y/2)-(other->pos.y-other->size.y/2));
	float collisionY2=((other->pos.y+other->size.y/2)-(pos.y-size.y/2));
	// collision only if on both axes
	if(collisionX1>0&&collisionX2>0&&collisionY1>0&&collisionY2>0) {
		if(std::abs(collisionX2-collisionX1)/size.x>std::abs(collisionY2-collisionY1)/size.y) {
			Vector2 vec(collisionX2-collisionX1, 0.0f);
			return CollitionData(vec.normalized(), std::min(collisionX1, collisionX2));
		} else {
			Vector2 vec(0.0f, collisionY2-collisionY1);
			return CollitionData(vec.normalized(), std::min(collisionY1, collisionY2));
		}
	} else {
		return CollitionData(Vector2(0.0f, 0.0f), 0.0f);
	}
}
#pragma endregion// BoxCollider

int main(int argc, char** argv) {
	// setup engine
	engine=new Engine(Vector2(1920.0, 1080.0), "Ghost Game", false);
	if(!engine->initialized||engine->ended) {
		Log("Engine failed to init");
		return 0;
	}
	// setup fps tracker
	tracker=new FpsTracker(engine);
	lightStencil=StencilSimple();
	// setup cameras
	cam=new OrthoCam(engine, Vector2(0.0f, 0.0f), Vector2(480.0f, 270.0f));
	uiCam=new OrthoCam(engine, Vector2(480.0f, 270.0f), Vector2(960.0f, 540.0f));
	if(engine->ended||!cam->initialized||!uiCam->initialized) {
		Log("Cameras failed to init");
		engine->Delete();
		return 0;
	}
	// load map data
	loadMapData("map");
	if(!parsedMap) {
		Log("Map data failed to load");
		engine->Delete();
		return 0;
	}
	// setup textures
	Texture playerTex=Texture(engine, playerTexPath);
	Texture flashlightTex=Texture(engine, flashlightTexPath);
	Texture backgroundTex=Texture(engine, mapTexPath);
	Texture minimapTex=Texture(engine, minimapTexPath);
	Texture instanceUnlitTex=Texture(engine, instanceUnlitTexPath);
	Texture instanceWorkingTex=Texture(engine, instanceWorkingTexPath);
	Texture instanceBrokenTex=Texture(engine, instanceBrokenTexPath);
	if(engine->ended||!playerTex.initialized||!flashlightTex.initialized||
		!backgroundTex.initialized||!minimapTex.initialized||
		!instanceUnlitTex.initialized||!instanceWorkingTex.initialized||
		!instanceBrokenTex.initialized
		) {
		Log("Textures failed to init");
		engine->Delete();
		return 0;
	}
	minimapSize=Vector2((float)minimapTex.width, (float)minimapTex.height)*minimapScale;
#pragma region setup shaders
	playerShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	flashlightShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	playerIconShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	backgroundShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	minimapShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	instanceUnlitShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	instanceWorkingShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	instanceBrokenShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	lineShader=new Shader(engine, "Shaders/vs.glsl", "Shaders/colorFrag.glsl");
	textShader=new Shader(engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	if(engine->ended||
		!playerShader->initialized||!flashlightShader->initialized||
		!playerIconShader->initialized||!backgroundShader->initialized||
		!minimapShader->initialized||!instanceUnlitShader->initialized||
		!instanceWorkingShader->initialized||!instanceBrokenShader->initialized||
		!lineShader->initialized||!textShader->initialized
		) {
		Log("Shaders failed to init");
		engine->Delete();
		return 0;
	}
	// set shader constants
	playerShader->setTexture("_texture", &playerTex, 0);
	playerShader->setFloat4("modulate", Vector4(playerModulate, 1.0f));
	flashlightShader->setTexture("_texture", &flashlightTex, 0);
	flashlightShader->setFloat4("modulate", Vector4(flashlightColor, 0.25f));
	playerIconShader->setTexture("_texture", &playerTex, 0);
	playerIconShader->setFloat4("modulate", Vector4(playerModulate, 0.75f));
	backgroundShader->setTexture("_texture", &backgroundTex, 0);
	minimapShader->setTexture("_texture", &minimapTex, 0);
	minimapShader->setFloat4("modulate", Vector4(1.0f, 1.0f, 1.0f, 0.75f));
	instanceUnlitShader->setTexture("_texture", &instanceUnlitTex, 0);
	instanceUnlitShader->setFloat4("modulate", Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	instanceWorkingShader->setTexture("_texture", &instanceWorkingTex, 0);
	instanceBrokenShader->setTexture("_texture", &instanceBrokenTex, 0);
	lineShader->setFloat4("color", Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	cam->bindShader(playerShader);
	cam->bindShader(flashlightShader);
	uiCam->bindShader(playerIconShader);
	cam->bindShader(backgroundShader);
	uiCam->bindShader(minimapShader);
	cam->bindShader(instanceUnlitShader);
	cam->bindShader(instanceWorkingShader);
	cam->bindShader(instanceBrokenShader);
	cam->bindShader(lineShader);
	uiCam->bindShader(textShader);
	cam->use();
	uiCam->use();
#pragma endregion// setup shaders
	// player and playerIcon
	playerRenderer=new SpriteRenderer(engine, playerShader, Vector2(), playerSize*mapScale);
	sceneRenderers.push_back(playerRenderer);
	flashlightRenderer=new SpriteRenderer(engine, flashlightShader, Vector2(), flashlightRange*mapScale, 1.0f);
	playerIconRenderer=new SpriteRenderer(engine, playerIconShader, Vector2(), Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), 1.0f);
	uiRenderers.push_back(playerIconRenderer);
	// map and minimap
	sceneRenderers.push_back(new SpriteRenderer(engine, backgroundShader, fullMapSize/2.0f, fullMapSize));// background
	uiRenderers.push_back(new SpriteRenderer(engine, minimapShader, Vector2(minimapSize.x/2.0f, 540.0f-minimapSize.y/2.0f), minimapSize));// minimap
	// setup text renderers
	debugText.push_back(new TextRenderer(engine, textShader, "Pos: ", Vector2(1.0f, 33.0f), 2.0f, Vector3(0.75f, 0.75f, 0.75f)));
	debugText.push_back(new TextRenderer(engine, textShader, "Fps Avg: ", Vector2(1.0f, 17.0f), 2.0f, Vector3(0.75f, 0.75f, 0.75f)));
	debugText.push_back(new TextRenderer(engine, textShader, "Time: ", Vector2(1.0f, 1.0f), 2.0f, Vector3(0.75f, 0.75f, 0.75f)));
	if(engine->ended||!characterMapInitialized) {
		Log("Fonts failed to init");
		engine->Delete();
		return 0;
	}
	// map data
	for(unsigned int i=0; i<instanceData.size(); i++) {// instances
		std::vector<int> dat=instanceData[i];
		Vector2 pos=gridToWorld(Vector2((float)dat[0]+0.5f, (float)dat[1]+0.5f));
		instanceRenderers.push_back(new SpriteRenderer(engine, instanceUnlitShader, pos, Vector2(mapScale, mapScale), 2.0f));
		bool broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
		instanceStateRenderers.push_back(new SpriteRenderer(engine, broken ? instanceBrokenShader : instanceWorkingShader, pos, Vector2(mapScale, mapScale), 3.0f));
		instanceColliders.push_back(new BoxCollider(engine, pos, Vector2(mapScale, mapScale), lineShader));
	}
	for(unsigned int i=0; i<horizontalWallData.size(); i++) {
		Vector3 line=horizontalWallData[i];
		instanceColliders.push_back(new BoxCollider(engine, gridToWorld(Vector2((line.z+line.y)/2, line.x)), Vector2((line.z-line.y)*(1.0f+spacing)*mapScale,spacing*4*mapScale), lineShader));
	}
	for(unsigned int i=0; i<verticalWallData.size(); i++) {
		Vector3 line=verticalWallData[i];
		instanceColliders.push_back(new BoxCollider(engine, gridToWorld(Vector2(line.x,(line.z+line.y)/2)), Vector2(spacing*4*mapScale,(line.z-line.y)*(1.0f+spacing)*mapScale), lineShader));
	}
	// setup other stuff
	playerCollider=new BoxCollider(engine, Vector2(), playerHitbox*playerSize*mapScale, lineShader);
	//ColliderDebug=true;// make hitboxes visible
	playerController=new PlayerController(engine, playerRenderer, playerCollider, flashlightRenderer, playerIconRenderer, cam);
	playerController->setPos(gridToWorld(playerOffset));
	// run main loop
	engine->onLoop.push_back(Loop);
	engine->onDelete.push_back(onLateDelete);
	Log("Engine initialized successfully");
	engine->Loop();
	return 1;
}
void Loop(double delta) {
	// set debug text
	Vector2 playerPos = playerController->getPos();
	debugText[0]->text="Pos: "+playerPos.to_string();
	debugText[1]->text="Fps Avg: "+std::to_string(tracker->getAvgFps())+", high: "+std::to_string(tracker->getHighFps())+", low: "+std::to_string(tracker->getLowFps());
	debugText[2]->text="Time: "+std::to_string(glfwGetTime());
	// draw scene
	for(Renderer* ren:sceneRenderers) ren->draw();
	for(SpriteRenderer* ren:instanceRenderers) if ((playerPos-ren->position).length() <= (sqrt(2)+7.7)*mapScale) ren->draw();
	
	lightStencil.Enable();
	lightStencil.Write();
	flashlightRenderer->draw();
	lightStencil.Compare();

	for(SpriteRenderer* ren:instanceStateRenderers) if ((playerPos-ren->position).length() <= (sqrt(2)+flashlightRange)*mapScale) ren->draw();
	lightStencil.Disable();
	playerCollider->drawOutline();
	for(BoxCollider* col:instanceColliders) if ((playerPos-col->pos).length() <= (sqrt(2)+7.7)*mapScale) col->drawOutline();
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* ren:uiRenderers) ren->draw();
	for(TextRenderer* ren:debugText) ren->draw();
}
void onLateDelete() {
	delete engine;
	delete tracker;
	delete playerController;

	delete cam;
	delete uiCam;

	delete playerShader;
	delete flashlightShader;
	delete playerIconShader;
	delete backgroundShader;
	delete instanceUnlitShader;
	delete instanceWorkingShader;
	delete instanceBrokenShader;
	delete minimapShader;
	delete lineShader;
	delete textShader;

	delete flashlightRenderer;

	for(Renderer* ren:sceneRenderers) { delete ren; }
	for(Renderer* ren:instanceRenderers) { delete ren; }
	for(SpriteRenderer* ren:instanceStateRenderers) { delete ren; }
	for(Renderer* ren:uiRenderers) { delete ren; }
	for(TextRenderer* ren:debugText) { delete ren; }
	sceneRenderers.clear();
	instanceRenderers.clear();
	instanceStateRenderers.clear();
	uiRenderers.clear();
	debugText.clear();

	delete playerCollider;
	for(BoxCollider* col:instanceColliders) { delete col; }
	instanceColliders.clear();
}