#include "Main.h"

#pragma region Shader creators
Shader* createTexShader(Texture* tex, Vector4 modulate) {
	Shader* shader=new Shader(engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	if(!shader->initialized) return shader;
	shader->setTexture("_texture", tex, 0);
	shader->setFloat4("modulate", modulate);
	return shader;
}
Shader* createColorShader(Vector4 color) {
	Shader* shader=new Shader(engine, "Shaders/vs.glsl", "Shaders/colorFrag.glsl");
	if(!shader->initialized) return shader;
	shader->setFloat4("color", color);
	return shader;
}
Shader* createTextShader() {
	Shader* shader=new Shader(engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	return shader;
}
#pragma endregion// Shader creators

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

#pragma region BoxCollider
BoxCollider::BoxCollider(Engine* _engine, Vector2 _pos, Vector2 _size, Shader* _debugLineShader) :
	LineRenderer(_engine, _debugLineShader, { Vector2(-_size.x/2, _size.y/2), Vector2(_size.x/2, _size.y/2), Vector2(_size.x/2, -_size.y/2), Vector2(-_size.x/2, -_size.y/2) }, 2.0f, _pos, true),
	size(_size), boundingRadius(sqrt(size.x*size.x+size.y*size.y)) {}
void BoxCollider::draw() {
	if(ColliderDebug) LineRenderer::draw();
}
CollitionData BoxCollider::checkCollision(BoxCollider* other) {
	if((position-other->position).length()-boundingRadius-other->boundingRadius>=0) CollitionData(Vector2(0.0f, 0.0f), 0.0f);
	if(position==other->position) return CollitionData(Vector2(0.0f, 0.0f), 0.0f);
	// collision x-axis?
	float collisionX1=((position.x+size.x/2)-(other->position.x-other->size.x/2));
	float collisionX2=((other->position.x+other->size.x/2)-(position.x-size.x/2));
	// collision y-axis?
	float collisionY1=((position.y+size.y/2)-(other->position.y-other->size.y/2));
	float collisionY2=((other->position.y+other->size.y/2)-(position.y-size.y/2));
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

#pragma region Player
Player::Player(Engine* _engine, OrthoCam* _sceneCam, Vector2 _position, Shader* playerShader, Shader* flashlightShader, Shader* iconShader)
	: Object(_engine), position(_position), flashlightStencil(StencilSimple()), sceneCam(_sceneCam), renderer(nullptr), collider(nullptr), flashlightRenderer(nullptr), iconRenderer(nullptr) {
	if(!initialized) return;

	renderer=new SpriteRenderer(engine, playerShader, position, playerSize*mapScale, 1);
	sceneRenderers.push_back(renderer);
	collider=new BoxCollider(engine, position, playerHitbox*playerSize*mapScale, lineShader);
	instanceColliders.push_back(collider);
	flashlightRenderer=new SpriteRenderer(engine, flashlightShader, position, flashlightRange*mapScale, 1.0f);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(worldToGrid(position)), Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), 1.0f);
	uiRenderers.push_back(iconRenderer);

	resolveCollitions();
	renderer->position=position;
	flashlightRenderer->position=position;
	sceneCam->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));
	sceneCam->update();
	sceneCam->use();

	engine->sub_key(this);
	engine->sub_loop(this);
	engine->sub_delete(this);
}
void Player::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(engine->ended||!initialized) return;
	if(key==GLFW_KEY_W) inputs[0]=action;
	else if(key==GLFW_KEY_A) inputs[1]=action;
	else if(key==GLFW_KEY_S) inputs[2]=action;
	else if(key==GLFW_KEY_D) inputs[3]=action;
	else if(key==GLFW_KEY_LEFT_SHIFT) inputs[4]=action;
}
void Player::on_loop(double delta) {
	if(engine->ended||!initialized) return;
	Vector2 inputVec=Vector2(
		(float)(inputs[3]>=GLFW_PRESS)-(float)(inputs[1]>=GLFW_PRESS),
		(float)(inputs[0]>=GLFW_PRESS)-(float)(inputs[2]>=GLFW_PRESS)
	).normalized();
	if(inputVec.x==0&&inputVec.y==0) return;
	position+=inputVec*((float)delta)*((inputs[4]>=GLFW_PRESS) ? playerSprintSpeed : playerSpeed)*mapScale*(1+spacing);
	resolveCollitions();
	renderer->position=position;
	flashlightRenderer->position=position;
	sceneCam->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
void Player::on_delete() {
	if(!initialized) return;
	delete flashlightRenderer;
}
void Player::resolveCollitions() {
	if(engine->ended||!initialized) return;
	collider->position=position;
	for(unsigned int i=0; i<instanceColliders.size(); i++) {
		CollitionData collition=instanceColliders[i]->checkCollision(collider);
		collider->position+=collition.normal*collition.dist;
	}
	position=collider->position;
}
void Player::flashlightStencilOn() {
	if(engine->ended||!initialized) return;
	flashlightStencil.Enable();
	flashlightStencil.Write();
	flashlightRenderer->draw();
	flashlightStencil.Compare();
}
void Player::flashlightStencilOff() {
	if(engine->ended||!initialized) return;
	flashlightRenderer->draw();
	flashlightStencil.Disable();
}
void Player::setPos(Vector2 _position) {
	if(engine->ended||!initialized) return;
	position=_position;
	resolveCollitions();
	renderer->position=position;
	flashlightRenderer->position=position;
	sceneCam->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
#pragma endregion// Player

#pragma region Enemy
Enemy::Enemy(Engine* _engine, Vector2 _position, Shader* enemyShader, Shader* iconShader)
	: Object(_engine), position(_position), renderer(nullptr), collider(nullptr), iconRenderer(nullptr) {
	if(!initialized) return;

	renderer=new SpriteRenderer(engine, enemyShader, position, playerSize*mapScale, 1);
	sceneRenderers.push_back(renderer);
	collider=new BoxCollider(engine, position, playerHitbox*playerSize*mapScale, lineShader);
	instanceColliders.push_back(collider);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(worldToGrid(position)), Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), 1.0f);
	uiRenderers.push_back(iconRenderer);

	resolveCollitions();
	renderer->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));

	engine->sub_key(this);
	engine->sub_loop(this);
}
void Enemy::on_loop(double delta) {
	if(engine->ended||!initialized) return;
	position+=Vector2(1.0f, 0.0f)*((float)delta)*mapScale*(1+spacing);
	resolveCollitions();
	renderer->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));
}
void Enemy::resolveCollitions() {
	if(engine->ended||!initialized) return;
	collider->position=position;
	for(unsigned int i=0; i<instanceColliders.size(); i++) {
		CollitionData collition=instanceColliders[i]->checkCollision(collider);
		collider->position+=collition.normal*collition.dist;
	}
	position=collider->position;
}
void Enemy::setPos(Vector2 _position) {
	if(engine->ended||!initialized) return;
	position=_position;
	resolveCollitions();
	renderer->position=position;
	iconRenderer->position=gridToMinimap(worldToGrid(position));
}
#pragma endregion// Enemy

Vector2 HD1080P(1920.0, 1080.0);
Vector2 viewRange(480.0f, 270.0f);
int main(int argc, char** argv) {
	// setup engine
	engine=new Engine(HD1080P, "Ghost Game", false);
	if(!engine->initialized||engine->ended) {
		Log("Engine failed to init.");
		return 0;
	}
	// setup fps tracker
	tracker=new FpsTracker(engine);
	// setup cameras
	cam=new OrthoCam(engine, Vector2(), viewRange);
	uiCam=new OrthoCam(engine, Vector2(480.0f, 270.0f), Vector2(960.0f, 540.0f));
	if(engine->ended||!cam->initialized||!uiCam->initialized) {
		Log("Cameras failed to init.");
		engine->Delete();
		return 0;
	}
	// load map data
	loadMapData("map");
	if(!parsedMap) {
		Log("Map data failed to load.");
		engine->Delete();
		return 0;
	}
	// setup textures
	Texture playerTex=Texture(engine, playerTexPath);
	Texture enemyTex=Texture(engine, enemyTexPath);
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
		Log("Textures failed to init.");
		engine->Delete();
		return 0;
	}
	minimapSize=Vector2((float)minimapTex.width, (float)minimapTex.height)*minimapScale;
	// setup shaders
	playerShader=createTexShader(&playerTex, Vector4(playerModulate, 1.0f));
	flashlightShader=createTexShader(&flashlightTex, Vector4(flashlightColor, 0.25f));
	playerIconShader=createTexShader(&playerTex, Vector4(playerModulate, 0.75f));

	enemyShader=createTexShader(&enemyTex, Vector4(enemyModulate, 1.0f));
	enemyIconShader=createTexShader(&enemyTex, Vector4(enemyModulate, 0.75f));

	backgroundShader=createTexShader(&backgroundTex, Vector4());
	minimapShader=createTexShader(&minimapTex, Vector4(1.0f, 1.0f, 1.0f, 0.75f));

	instanceUnlitShader=createTexShader(&instanceUnlitTex, Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	instanceWorkingShader=createTexShader(&instanceWorkingTex, Vector4());
	instanceBrokenShader=createTexShader(&instanceBrokenTex, Vector4());

	lineShader=createColorShader(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	textShader=createTextShader();
	if(engine->ended||
		!playerShader->initialized||!flashlightShader->initialized||
		!playerIconShader->initialized||!backgroundShader->initialized||
		!minimapShader->initialized||!instanceUnlitShader->initialized||
		!instanceWorkingShader->initialized||!instanceBrokenShader->initialized||
		!lineShader->initialized||!textShader->initialized
		) {
		Log("Shaders failed to init.");
		engine->Delete();
		return 0;
	}
	cam->bindShaders({ playerShader, flashlightShader, backgroundShader, enemyShader, instanceUnlitShader, instanceWorkingShader, instanceBrokenShader, lineShader });
	uiCam->bindShaders({ minimapShader, playerIconShader, enemyIconShader, textShader });
	cam->use();
	uiCam->use();
	// player object
	enemy=new Enemy(engine, gridToWorld(playerOffset+Vector2(0.0f, 2.0f)), enemyShader, enemyIconShader);
	player=new Player(engine, cam, gridToWorld(playerOffset), playerShader, flashlightShader, playerIconShader);
	// map and minimap
	sceneRenderers.push_back(new SpriteRenderer(engine, backgroundShader, fullMapSize/2.0f, fullMapSize));// background
	uiRenderers.push_back(new SpriteRenderer(engine, minimapShader, Vector2(minimapSize.x/2.0f, 540.0f-minimapSize.y/2.0f), minimapSize));// minimap
	// setup text renderers
	debugText.push_back(new TextRenderer(engine, textShader, "Pos:\nFps Avg:\nTime:", Vector2(1.0f, 36.0f), 2.0f, Vector3(0.75f, 0.75f, 0.75f)));
	if(engine->ended||!characterMapInitialized) {
		Log("Fonts failed to init.");
		engine->Delete();
		return 0;
	}
	//ColliderDebug=true;// make hitboxes visible
	// create instances
	for(unsigned int i=0; i<instanceData.size(); i++) {
		std::vector<int> dat=instanceData[i];
		Vector2 pos=gridToWorld(Vector2((float)dat[0]+0.5f, (float)dat[1]+0.5f));
		sceneRenderers.push_back(new SpriteRenderer(engine, instanceUnlitShader, pos, Vector2(mapScale, mapScale), 2.0f));
		bool broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
		instanceStateRenderers.push_back(new SpriteRenderer(engine, broken ? instanceBrokenShader : instanceWorkingShader, pos, Vector2(mapScale, mapScale), 3.0f));
		instanceColliders.push_back(new BoxCollider(engine, pos, Vector2(mapScale, mapScale), lineShader));
	}
	// create horizontal wall colliders
	for(unsigned int i=0; i<horizontalWallData.size(); i++) {
		Vector3 line=horizontalWallData[i];
		instanceColliders.push_back(new BoxCollider(engine, gridToWorld(Vector2((line.z+line.y)/2, line.x)), Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*4)*mapScale, spacing*4*mapScale), lineShader));
	}
	// create vertical wall colliders
	for(unsigned int i=0; i<verticalWallData.size(); i++) {
		Vector3 line=verticalWallData[i];
		instanceColliders.push_back(new BoxCollider(engine, gridToWorld(Vector2(line.x, (line.z+line.y)/2)), Vector2(spacing*4*mapScale, ((line.z-line.y)*(1.0f+spacing)+spacing*4)*mapScale), lineShader));
	}
	// run main loop
	engine->onLoop.push_back(Loop);
	engine->onDelete.push_back(onLateDelete);
	Log("Engine initialized successfully.");
	engine->Loop();
	return 1;
}
void Loop(double delta) {
	// set debug text
	Vector2 playerPos=player->position;
	debugText[0]->text="Pos: "+playerPos.to_string()+"\n";
	debugText[0]->text+="Fps Avg: "+std::to_string(tracker->getAvgFps())+", high: "+std::to_string(tracker->getHighFps())+", low: "+std::to_string(tracker->getLowFps())+"\n";
	debugText[0]->text+="Time: "+std::to_string(glfwGetTime());
	// draw scene
	for(Renderer2D* ren:sceneRenderers) if(ren->shouldDraw(playerPos, viewRange)) ren->draw();
	player->flashlightStencilOn();
	for(Renderer2D* ren:instanceStateRenderers) if(ren->shouldDraw(playerPos, flashlightRange*mapScale)) ren->draw();
	player->flashlightStencilOff();

	for(BoxCollider* col:instanceColliders) if(col->shouldDraw(playerPos, viewRange)) col->draw();
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* ren:uiRenderers) ren->draw();
	for(TextRenderer* ren:debugText) ren->draw();
}
void onLateDelete() {
	delete engine;
	delete tracker;
	delete player;
	delete enemy;
	//cams
	delete cam;
	delete uiCam;
	//shaders
	delete playerShader;
	delete flashlightShader;
	delete playerIconShader;

	delete enemyShader;
	delete enemyIconShader;

	delete backgroundShader;
	delete minimapShader;

	delete instanceUnlitShader;
	delete instanceWorkingShader;
	delete instanceBrokenShader;

	delete lineShader;
	delete textShader;
	//renderers
	for(Renderer2D* ren:sceneRenderers) { delete ren; }
	for(Renderer2D* ren:instanceStateRenderers) { delete ren; }
	for(Renderer* ren:uiRenderers) { delete ren; }
	for(TextRenderer* ren:debugText) { delete ren; }
	sceneRenderers.clear();
	instanceStateRenderers.clear();
	uiRenderers.clear();
	debugText.clear();

	for(BoxCollider* col:instanceColliders) { delete col; }
	instanceColliders.clear();
}