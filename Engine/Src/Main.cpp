#include "Main.h"
#include "EngineLib.h"

#ifdef _DEBUG
#include <memory>
unsigned int allocated=0;
void* operator new(std::size_t size) throw(std::bad_alloc) {
	allocated+=static_cast<unsigned int>(size);
	//std::cout << "Allocated: " << size << " bytes.\n";
	return malloc(size);
}
void operator delete(void* memory, std::size_t size) throw() {
	allocated-=static_cast<unsigned int>(size);
	//std::cout << "Deallocated: " << size << " bytes.\n";
	free(memory);
}
#endif// _DEBUG

#pragma region Player
void Player::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(engine->ended||!initialized) return;
	if(key==GLFW_KEY_W) inputs[0]=action;
	else if(key==GLFW_KEY_A) inputs[1]=action;
	else if(key==GLFW_KEY_S) inputs[2]=action;
	else if(key==GLFW_KEY_D) inputs[3]=action;
	else if(key==GLFW_KEY_LEFT_SHIFT) inputs[4]=action;
}
void Player::on_loop(const double& delta) {
	if(engine->ended||!initialized) return;
	Vector2 inputVec=Vector2(
		(float)(inputs[3]>=GLFW_PRESS)-(float)(inputs[1]>=GLFW_PRESS),
		(float)(inputs[0]>=GLFW_PRESS)-(float)(inputs[2]>=GLFW_PRESS)
	).normalized();
	if(inputVec.x==0&&inputVec.y==0) return;
	inputVec=inputVec*((float)delta)*((inputs[4]!=GLFW_RELEASE) ? playerSprintSpeed : playerSpeed)*mapScale*(1+spacing);
	transform.position+=collider->tryMove(inputVec, MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
	sceneCam->update();
	sceneCam->use();
}
Player::Player(OrthoCam* _sceneCam, const Vector3& playerModulate, Texture* playerTex, const Vector4& flashlightColor, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale)), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(_sceneCam), flashlightRenderer(nullptr), iconRenderer(nullptr) {
	if(!initialized) return;
	addChild(sceneCam);
	sceneCam->transform.position=Vector2::ZERO;
	renderer=spriteRenderer->addSprite(Vector4(playerModulate, 1.0f), playerTex, Vector2::ZERO, 0.0f, Vector2(playerSize));
	addChild(renderer);
	collider=new BoxCollider(lineShader, PLAYERMASK, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(&collider->transform);
	flashlightRenderer=new DotRenderer(createDotColorShader(flashlightColor), flashlightRange, Vector2::ZERO, 1.0f);
	addChild(flashlightRenderer);
	iconRenderer=uiHandler->Sprite(Vector4(playerModulate, 0.75f), playerTex, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y));
	transform.position+=collider->forceOut(MAPMASK|ENEMYMASK);
	sceneCam->update();
	sceneCam->use();

	Engine::instance->sub_key(this);
	Engine::instance->sub_loop(this);
}
void Player::flashlightStencilOn() {
	if(Engine::instance->ended||!initialized) return;
	flashlightStencil.Enable();
	flashlightStencil.Write();
	flashlightRenderer->draw();
	flashlightStencil.Compare();
}
void Player::flashlightStencilOff() {
	if(Engine::instance->ended||!initialized) return;
	flashlightRenderer->draw();
	flashlightStencil.Disable();
}
void Player::setPos(const Vector2& _position) {
	if(Engine::instance->ended||!initialized) return;
	setWorldPos(_position);
	transform.position+=collider->forceOut(MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
	sceneCam->update();
	sceneCam->use();
}
#pragma endregion// Player

#pragma region Enemy
void Enemy::setDebugLine(std::vector<Vector2> line) {
	if(Engine::instance->ended||!initialized) return;
	delete debugRen;
	if(line.size()>0) {
		std::vector<Vector2> renderLine;
		for(const Vector2& pos:line) renderLine.push_back(pos);
		renderLine.push_back(getWorldPos());
		debugRen=new LineRenderer(lineShader, renderLine, 3, false, Vector2::ZERO, 100.0f);
	} else debugRen=nullptr;
}
void Enemy::on_loop(const double& delta) {
	if(Engine::instance->ended||!initialized) return;
	Vector2 targetPos=Pathfinder::WorldToGrid(target->getWorldPos());
	if(lastSpottedPos!=targetPos) {
		Vector2 worldPos=getWorldPos();
		BoxCollider::CollitionData hit=collider->raycast(worldPos, target->getWorldPos(), MAPMASK);
		if(!((bool)hit)) {
			//double startTime=glfwGetTime();
			path=pathfinder->pathfind(Pathfinder::WorldToGrid(worldPos), targetPos);
			//Log("Final Time: "+std::to_string((glfwGetTime()-startTime)*1000.0)+"ms");
			lastSpottedPos=targetPos;
		}// else setDebugLine({ hit.point });
	}

	float travelDist=enemySpeed*((float)delta)*mapScale*(1+spacing);
	while(path.size()>0&&travelDist>0) {
		Vector2 worldPos=getWorldPos();
		if((worldPos-target->getWorldPos()).length()<=mapScale*(1+spacing)) { path.clear();break; }
		Vector2 dir=path.back()-worldPos;
		float dist=dir.length();
		if(travelDist<dist) {
			transform.position+=dir.normalized()*travelDist;
			break;
		} else {// travelDist>dist
			transform.position+=dir;
			travelDist-=dist;
			path.pop_back();
			continue;
		}
	}
	//setDebugLine(path);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
}
Enemy::Enemy(const Vector3& enemyModulate, Texture* enemyTex, Shader* _lineShader, Pathfinder* _pathfinder, hasTransform2D* _target, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale), Vector2::Center, 0.0f), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(_lineShader), pathfinder(_pathfinder), target(_target) {
	if(!initialized) return;
	renderer=spriteRenderer->addSprite(Vector4(enemyModulate, 1.0f), enemyTex, Vector2::ZERO, 0.0f, Vector2(playerSize));
	addChild(renderer);
	collider=new BoxCollider(lineShader, ENEMYMASK, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(&collider->transform);
	iconRenderer=uiHandler->Sprite(Vector4(enemyModulate, 0.75f), enemyTex, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), Vector2::Center);
	engine->sub_loop(this);
}
#pragma endregion// Enemy

#pragma region Instance
void Instance::on_click(const Vector2& pos) {
	if(broken&&(player->getWorldPos()-pos).sqrMagnitude()<=(std::pow(flashlightRange*mapScale/2.0f, 2)))
		fixInstance();
}
Instance::Instance(ClickDetector* clickDetector, Shader* lineShader, Texture* _instanceUnlitTex, Texture* _instanceWorkingTex, Texture* _instanceBrokenTex, const Vector2& _position, const Vector2& _anchor, const float& _rotAngle) :
	Clickable(clickDetector), hasTransform2D(_position, 0.0f, Vector2(mapScale), _anchor, _rotAngle), instanceWorkingTex(_instanceWorkingTex), instanceBrokenTex(_instanceBrokenTex) {
	broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
	addChild(staticSpriteRenderer->addSprite(Vector4(0.5f, 0.5f, 0.5f, 1.0f), _instanceUnlitTex, Vector2::ZERO, 1.0f));
	stateQuad=instanceStateSpritesRenderer->addSprite(Vector4::ONE, broken ? instanceBrokenTex : instanceWorkingTex, Vector2::ZERO, 2.0f);
	addChild(stateQuad);
	addChild(new BoxCollider(lineShader, MAPMASK, Vector2::ZERO, 100.0f));
}
void Instance::fixInstance() {
	broken=false;
	stateQuad->tex=instanceWorkingTex;
	instanceStateSpritesRenderer->bind();
}
void Instance::breakInstance() {
	broken=true;
	stateQuad->tex=instanceBrokenTex;
	instanceStateSpritesRenderer->bind();
}
#pragma endregion// Instance

#define VSYNC false
int Run() {
#pragma region Engine setup
	// load map data
	loadMapData("map");
	if(!parsedMap) {
		Log("Map data failed to load.");
		return 0;
	}
	// setup Engine
	engine=new Engine(HD1080P, "Ghost Game", VSYNC);
	if(!engine->initialized||engine->ended) {
		Log("Engine failed to init.");
		return 0;
	}
	//not required but I do this
	if(!TextRenderer::initCharacterMap()) {
		Log("Error initializing font.");
		engine->Delete();
		return 0;
	}
	// setup cameras
	cam=new OrthoCam(Vector2::ZERO, viewRange);
	uiCam=new OrthoCam(viewRange, viewRange*2.0f);
	if(engine->ended||!cam->initialized||!uiCam->initialized) {
		Log("Cameras failed to init.");
		engine->Delete();
		return 0;
	}
	// setup textures
	Texture* playerTex=new Texture(playerTexPath);
	Texture* enemyTex=new Texture(enemyTexPath);

	Texture* backgroundTex=new Texture(mapTexPath);
	Texture* minimapTex=new Texture(minimapTexPath);

	Texture* instanceUnlitTex=new Texture(instanceUnlitTexPath);
	Texture* instanceWorkingTex=new Texture(instanceWorkingTexPath);
	Texture* instanceBrokenTex=new Texture(instanceBrokenTexPath);
	if(engine->ended||
		!playerTex->initialized||!enemyTex->initialized||
		!backgroundTex->initialized||!minimapTex->initialized||
		!instanceUnlitTex->initialized||!instanceWorkingTex->initialized||!instanceBrokenTex->initialized
		) {
		Log("Textures failed to init.");
		engine->Delete();
		return 0;
	}
	minimapSize=Vector2((float)minimapTex->width, (float)minimapTex->height)*minimapScale;
	// setup shaders
	lineShader=createColorShader(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	if(engine->ended||!lineShader->initialized) { Log("Shaders failed to init."); engine->Delete(); return 0; }
	cam->bindShader(lineShader); cam->use();
#pragma endregion// Engine setup
	spriteRenderer=new BatchedSpriteRenderer(cam);
	staticSpriteRenderer=new StaticBatchedSpriteRenderer(cam);
	instanceStateSpritesRenderer=new StaticBatchedSpriteRenderer(cam);
	uiHandler=new UiHandler(uiCam);
	// player object
	player=new Player(cam, playerModulate, playerTex, flashlightColor, GridToWorld(playerOffset));
	finder=std::make_unique<Pathfinder>();
	enemy=new Enemy(enemyModulate, enemyTex, lineShader, finder.get(), player, GridToWorld(playerOffset-Vector2(0.0f, 2.0f)));
#pragma region Map setup
	staticSpriteRenderer->addSprite(Vector4::ZERO, backgroundTex, Vector2::ZERO, 0.0f, fullMapSize, Vector2::BottomLeft);// map background
	//ColliderDebug=true;// makes hitboxes visible
	instanceClickDetector=new ClickDetector(cam);
	for(const std::array<int, 5>&dat:instanceData) {
		Vector2 pos=GridToWorld(Vector2((float)dat[0], (float)dat[1])+Vector2(0.5f, 0.5f));
		new Instance(instanceClickDetector, lineShader, instanceUnlitTex, instanceWorkingTex, instanceBrokenTex, pos, Vector2::Center, 0.0f);
	}
	staticSpriteRenderer->bind();
	instanceStateSpritesRenderer->bind();
	for(const Vector3& line:horizontalWallData) {// horizontal walls
		new BoxCollider(lineShader, MAPMASK, GridToWorld(Vector2((line.z+line.y)/2.0f, line.x)), 100.0f, Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*3.0f), spacing*3.0f)*mapScale);
	}
	for(const Vector3& line:verticalWallData) {// vertical walls
		new BoxCollider(lineShader, MAPMASK, GridToWorld(Vector2(line.x, (line.z+line.y)/2.0f)), 100.0f, Vector2(spacing*3.0f, ((line.z-line.y)*(1.0f+spacing)+spacing*3.0f))*mapScale);
	}
#pragma endregion// Map setup
	// setup UI
	uiHandler->Sprite(Vector4(Vector3::ONE, 0.75f), minimapTex, Vector2(0.0f, viewRange.y*2.0f), 0.0f, minimapSize, Vector2::TopLeft);// minimap
	Button* testButton=uiHandler->createButton(
		Vector4(0.125f, 0.125f, 0.125f, 1.0f), Vector4(0.1f, 0.1f, 0.1f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f),
		Vector2(viewRange.x*2.0f, 0.0f), 0.0f, Vector2(25.0f, 25.0f), Vector2::BottomRight);
	testButton->addChild(uiHandler->Text("X", Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(6.0f, 4.0f), 15.0f, 2.0f, Vector2::BottomRight));
	testButton->onrelease=close;
	TextInput* testInput=uiHandler->createTextInput("", "Type here.",
		Vector2(viewRange.x*2.0f-25.0f, 0.0f), 0.0f, Vector2(400.0f, 25.0f), Vector2::BottomRight);

	fpsText=uiHandler->Text("Fps Avg:,high:,low:", Vector4(0.75f, 0.25f, 0.25f, 1.0f), viewRange*2.0f+Vector2(-464.0f, -1.0f), 15.0f, 2.0f, Vector2::TopLeft);;
#ifdef _DEBUG
	debugText=uiHandler->Text("Pos:\nTime:\nMemory:\nDraw calls:", Vector4(0.75f, 0.75f, 0.75f, 1.0f), Vector2(1.0f, 1.0f), 15.0f, 2.0f, Vector2::BottomLeft);
#endif// _DEBUG
	// run main loop
	engine->renderLoop=Loop;
	Log("Engine initialized successfully.");
	engine->Start();
	delete engine;
	colliders.clear();
	return 1;
}
void Loop(const double& delta) {
	fpsText->text="Fps Avg: "+std::to_string(engine->fpsAvg)+", high: "+std::to_string(engine->fpsHigh)+", low: "+std::to_string(engine->fpsLow);
#ifdef _DEBUG
	// set debug text
	debugText->text="Pos: "+player->getWorldPos().to_string()+"\nTime: "+std::to_string(glfwGetTime())+"\nMemory: "+std::to_string(allocated)+" bytes\nDraw calls: "+std::to_string(engine->drawCalls);
#endif// _DEBUG
	// draw scene
	spriteRenderer->draw();
	staticSpriteRenderer->draw();
	player->flashlightStencilOn();
	instanceStateSpritesRenderer->draw();
	player->flashlightStencilOff();
	//debug stuff
	if(enemy->debugRen) enemy->debugRen->draw();
	if(ColliderDebug) {
		for(BoxCollider* col:colliders) if(col->inRange(cam->getWorldPos(), cam->scale)) col->draw();
		player->collider->draw();
	}
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	uiHandler->draw();
}
void close() { engine->Close(); }

int main(int argc, char** argv) {
	int value=Run();
#ifdef _DEBUG
	Log("Press enter to close . . .");
	std::cin.get();
#endif// _DEBUG
	return value;
}