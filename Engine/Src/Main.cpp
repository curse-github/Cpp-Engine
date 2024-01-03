#include "Main.h"
#include "EngineLib.h"

#ifdef _DEBUG
#include <memory>
unsigned int allocated=0;
void* operator new(size_t size) {
	allocated+=size;
	//std::cout << "Allocated: " << size << " bytes.\n";
	return malloc(size);
}
void operator delete(void* memory, size_t size) {
	allocated-=size;
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
	inputVec=inputVec*((float)delta)*((inputs[4]>=GLFW_PRESS) ? playerSprintSpeed : playerSpeed)*mapScale*(1+spacing);
	transform.position+=collider->tryMove(inputVec, MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
	sceneCam->update();
	sceneCam->use();
}
Player::Player(OrthoCam* _sceneCam, Shader* playerShader, Shader* flashlightShader, Shader* iconShader, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale)), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(_sceneCam), flashlightRenderer(nullptr), iconRenderer(nullptr) {
	if(!initialized) return;
	addChild(sceneCam);
	sceneCam->transform.position=Vector2::ZERO;
	renderer=new SpriteRenderer(playerShader, Vector2::ZERO, 1.0f, Vector2(playerSize));
	sceneRenderers.push_back(renderer);
	addChild(renderer);
	collider=new BoxCollider(lineShader, PLAYERMASK, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(collider);
	flashlightRenderer=new SpriteRenderer(flashlightShader, Vector2::ZERO, 1.0f, flashlightRange);
	addChild(flashlightRenderer);
	iconRenderer=new SpriteRenderer(iconShader, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y));
	uiRenderers.push_back(iconRenderer);

	transform.position+=collider->forceOut(MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
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
Enemy::Enemy(Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, hasTransform2D* _target, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale), Vector2::Center, 0.0f), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(_lineShader), pathfinder(_pathfinder), target(_target) {
	if(!initialized) return;
	renderer=new SpriteRenderer(enemyShader, Vector2::ZERO, 1.0f, Vector2(playerSize));
	sceneRenderers.push_back(renderer);
	addChild(renderer);
	collider=new BoxCollider(lineShader, ENEMYMASK, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(collider);
	iconRenderer=new SpriteRenderer(iconShader, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), Vector2::Center);
	uiRenderers.push_back(iconRenderer);

	engine->sub_loop(this);
}
#pragma endregion// Enemy

#pragma region Instance
void Instance::on_click(const Vector2& pos) {
	if(broken) fixInstance();
}
Instance::Instance(OrthoCam* _cam, Shader* lineShader, StaticBatchedSpriteRenderer* _instanceRenderer, StaticBatchedSpriteRenderer* _instanceStateRenderer, const Vector2& _position, const Vector2& _anchor, const float& _rotAngle) :
	Clickable(_cam), hasTransform2D(_position, 0.0f, Vector2(mapScale), _anchor, _rotAngle), instanceStateRenderer(_instanceStateRenderer) {
	broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
	addChild(instanceRenderer->addQuad(Vector4(0.5f, 0.5f, 0.5f, 1.0f), 0.0f, Vector2::ZERO, 2.0f));
	addChild(stateQuad=instanceStateRenderer->addQuad(Vector4::ONE, broken ? 1.0f : 0.0f, Vector2::ZERO, 3.0f));
	addChild(new BoxCollider(lineShader, MAPMASK, Vector2::ZERO, 100.0f));
}
void Instance::fixInstance() {
	broken=false;
	stateQuad->texIndex=0.0f;
	instanceStateRenderer->bind();
}
void Instance::breakInstance() {
	broken=true;
	stateQuad->texIndex=1.0f;
	instanceStateRenderer->bind();
}
#pragma endregion// Instance

int Run() {
#pragma region Setup
	// load map data
	loadMapData("map");
	if(!parsedMap) {
		Log("Map data failed to load.");
		return 0;
	}
	// setup Engine
	engine=new Engine(HD1080P, "Ghost Game", false);
	if(!engine->initialized||engine->ended) {
		Log("Engine failed to init.");
		return 0;
	}
	// setup other stuff
	uiHandler=new UiHandler();
	tracker=new FpsTracker();
	finder=std::make_unique<Pathfinder>();
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
	Texture* flashlightTex=new Texture(flashlightTexPath);

	Texture* enemyTex=new Texture(enemyTexPath);

	Texture* backgroundTex=new Texture(mapTexPath);
	Texture* minimapTex=new Texture(minimapTexPath);

	Texture* instanceUnlitTex=new Texture(instanceUnlitTexPath);
	Texture* instanceWorkingTex=new Texture(instanceWorkingTexPath);
	Texture* instanceBrokenTex=new Texture(instanceBrokenTexPath);
	if(engine->ended||!playerTex->initialized||!flashlightTex->initialized||
		!enemyTex->initialized||!backgroundTex->initialized||
		!minimapTex->initialized||!instanceUnlitTex->initialized||
		!instanceWorkingTex->initialized||!instanceBrokenTex->initialized) {
		Log("Textures failed to init.");
		engine->Delete();
		return 0;
	}
	minimapSize=Vector2((float)minimapTex->width, (float)minimapTex->height)*minimapScale;
	// setup shaders
	playerShader=createTexShader(playerTex, Vector4(playerModulate, 1.0f));
	flashlightShader=createTexShader(flashlightTex, Vector4(flashlightColor, 0.25f));
	playerIconShader=createTexShader(playerTex, Vector4(playerModulate, 0.75f));

	enemyShader=createTexShader(enemyTex, Vector4(enemyModulate, 1.0f));
	enemyIconShader=createTexShader(enemyTex, Vector4(enemyModulate, 0.75f));

	backgroundShader=createTexShader(backgroundTex, Vector4::ZERO);
	minimapShader=createTexShader(minimapTex, Vector4(1.0f, 1.0f, 1.0f, 0.75f));

	instanceShader=createBatchedShader({ instanceUnlitTex });
	instanceStateShader=createBatchedShader({ instanceWorkingTex, instanceBrokenTex });

	lineShader=createColorShader(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	if(engine->ended||
		!playerShader->initialized||!flashlightShader->initialized||
		!playerIconShader->initialized||!enemyShader->initialized||
		!enemyIconShader->initialized||!backgroundShader->initialized||
		!minimapShader->initialized||!instanceShader->initialized||
		!instanceStateShader->initialized||!lineShader->initialized
		) {
		Log("Shaders failed to init.");
		engine->Delete();
		return 0;
	}
	cam->bindShaders({ playerShader, flashlightShader, enemyShader, backgroundShader, instanceShader, instanceStateShader, lineShader });
	uiCam->bindShaders({ playerIconShader, enemyIconShader, minimapShader });
	cam->use();
	uiCam->use();
#pragma endregion// Setup
	// player object
	player=new Player(cam, playerShader, flashlightShader, playerIconShader, GridToWorld(playerOffset));
	enemy=new Enemy(enemyShader, enemyIconShader, lineShader, finder.get(), player, GridToWorld(playerOffset-Vector2(0.0f, 2.0f)));
#pragma region Map setup
	// map background
	sceneRenderers.push_back(new SpriteRenderer(backgroundShader, Vector2::ZERO, 0.0f, fullMapSize, Vector2::BottomLeft));// background
	// create instances
	ColliderDebug=true;// make hitboxes visible
	instanceRenderer=new StaticBatchedSpriteRenderer(instanceShader);
	instanceStateRenderer=new StaticBatchedSpriteRenderer(instanceStateShader);
	for(const std::array<int, 5>&dat:instanceData) {
		Vector2 pos=GridToWorld(Vector2((float)dat[0], (float)dat[1])+Vector2(0.5f, 0.5f));
		new Instance(cam, lineShader, instanceRenderer, instanceStateRenderer, pos, Vector2::Center, 0.0f);
	}
	instanceRenderer->bind();
	instanceStateRenderer->bind();
	for(const Vector3& line:horizontalWallData) {// horizontal walls
		new BoxCollider(lineShader, MAPMASK, GridToWorld(Vector2((line.z+line.y)/2.0f, line.x)), 100.0f, Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*3.0f), spacing*3.0f)*mapScale);
	}
	for(const Vector3& line:verticalWallData) {// vertical walls
		new BoxCollider(lineShader, MAPMASK, GridToWorld(Vector2(line.x, (line.z+line.y)/2.0f)), 100.0f, Vector2(spacing*3.0f, ((line.z-line.y)*(1.0f+spacing)+spacing*3.0f))*mapScale);
	}
#pragma endregion// Map setup
	// setup UI
	uiRenderers.push_back(new SpriteRenderer(minimapShader, Vector2(0.0f, HD1080P.y/2.0f), 0.0f, minimapSize, Vector2::TopLeft));// minimap
	Button* testButton=new Button(uiCam,
		Vector4(0.125f, 0.125f, 0.125f, 1.0f), Vector4(0.1f, 0.1f, 0.1f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f),
		Vector2(viewRange.x*2.0f, 0.0f), 0.0f, Vector2(150.0f, 50.0f), Vector2::BottomRight);
	uiRenderers.push_back(testButton->renderer);
	testButton->onclick=close;
	// setup text renderers
	textRenderer=new BatchedTextRenderer(uiCam);
	fpsText=textRenderer->addText("Fps Avg:,high:,low:", Vector4(0.75f, 0.25f, 0.25f, 1.0f), viewRange+Vector2(-232.0f, -1.0f), 15.0f, 2.0f, Vector2::TopLeft);
#ifdef _DEBUG
	debugText=textRenderer->addText("Pos:\nTime:", Vector4(0.75f, 0.75f, 0.75f, 1.0f), Vector2(1.0f, 1.0f), 15.0f, 2.0f, Vector2::BottomLeft);
#endif// _DEBUG
	//BatchedTextData* test = textRenderer->addText("Button", Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(-37.5f, 12.5f), 15.0f, 2.0f, Vector2::Center);
	//testButton->addChild(test);
	if(engine->ended||!TextRenderer::characterMapInitialized) {
		Log("Fonts failed to init.");
		engine->Delete();
		return 0;
	}
	// run main loop
	engine->renderLoop=Loop;
	Log("Engine initialized successfully.");
	engine->Loop();
	delete engine;
	sceneRenderers.clear();
	uiRenderers.clear();
	colliders.clear();
	return 1;
}
void Loop(const double& delta) {
	fpsText->text="Fps Avg: "+std::to_string(tracker->getAvgFps())+", high: "+std::to_string(tracker->getHighFps())+", low: "+std::to_string(tracker->getLowFps());
#ifdef _DEBUG
	// set debug text
	debugText->text="Pos: "+player->getWorldPos().to_string()+"\nTime: "+std::to_string(glfwGetTime())+"\nMemory: "+std::to_string(allocated)+" bytes";
#endif// _DEBUG
	// draw scene
	for(Renderer2D* ren:sceneRenderers) if(ren->inRange(cam->getWorldPos(), cam->scale)) ren->draw();
	instanceRenderer->draw();
	player->flashlightStencilOn();
	instanceStateRenderer->draw();
	player->flashlightStencilOff();
	if(enemy->debugRen) enemy->debugRen->draw();
	if(ColliderDebug) {
		for(BoxCollider* col:colliders) if(col->inRange(cam->getWorldPos(), cam->scale)) col->draw();
		player->collider->draw();
	}
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* ren:uiRenderers) ren->draw();
	textRenderer->draw();
}
void close() {
	engine->Close();
}

int main(int argc, char** argv) {
	int value=Run();
#ifdef _DEBUG
	Log("Press enter to close . . .");
	std::cin.get();
#endif// _DEBUG
	return value;
}