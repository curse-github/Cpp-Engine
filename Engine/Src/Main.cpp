#include "Main.h"
#include "EngineLib.h"

#ifdef _DEBUG
#include <memory>
unsigned int allocatedPeak=0;
unsigned int allocated=0;
void* operator new(std::size_t size) throw(std::bad_alloc) {
	allocated+=static_cast<unsigned int>(size);
	if(allocated>allocatedPeak) allocatedPeak=allocated;// gets what heap memory peaked at during each frame
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
	if(key==GLFW_KEY_W) inputs[0]=action;
	else if(key==GLFW_KEY_A) inputs[1]=action;
	else if(key==GLFW_KEY_S) inputs[2]=action;
	else if(key==GLFW_KEY_D) inputs[3]=action;
	else if(key==GLFW_KEY_LEFT_SHIFT) inputs[4]=action;
}
void Player::on_loop(const double& delta) {
	Vector2 inputVec=Vector2(
		static_cast<float>(inputs[3]>=GLFW_PRESS)-static_cast<float>(inputs[1]>=GLFW_PRESS),
		static_cast<float>(inputs[0]>=GLFW_PRESS)-static_cast<float>(inputs[2]>=GLFW_PRESS)
	).normalized();
	if(inputVec.x==0&&inputVec.y==0) return;
	inputVec=inputVec*static_cast<float>(delta)*((inputs[4]!=GLFW_RELEASE) ? playerSprintSpeed : playerSpeed)*mapScale*(1+spacing);
	transform.position+=collider->tryMove(inputVec, MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
	sceneCam->update();
	sceneCam->use();
}
Player::Player(OrthoCam* _sceneCam, const Vector3& playerModulate, Texture* playerTex, const Vector4& flashlightColor, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale)), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(_sceneCam), flashlightRenderer(nullptr), iconRenderer(nullptr) {
	addChild(sceneCam);
	sceneCam->transform.position=Vector2::ZERO;
	renderer=spriteRenderer->addSprite(Vector4(playerModulate, 1.0f), playerTex, Vector2::ZERO, 2.0f, Vector2(playerSize));
	addChild(renderer);
	collider=new BoxCollider(PLAYERMASK, false, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(&collider->transform);
	flashlightRenderer=new DotRenderer(createDotColorShader(flashlightColor), flashlightRadius, Vector2::ZERO, 1.0f);
	_sceneCam->bindShader(flashlightRenderer->shader);
	addChild(flashlightRenderer);
	iconRenderer=uiHandler->Sprite(Vector4(playerModulate, 0.75f), playerTex, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y));
	transform.position+=collider->forceOut(MAPMASK|ENEMYMASK);
	sceneCam->update();
	sceneCam->use();

	Engine::instance->sub_key(this);
	Engine::instance->sub_loop(this);
}
void Player::flashlightStencilOn() {
	flashlightStencil.Enable();
	flashlightStencil.Write();
	flashlightRenderer->draw();
	flashlightStencil.Compare();
}
void Player::flashlightStencilOff() {
	flashlightRenderer->draw();
	flashlightStencil.Disable();
}
void Player::setPos(const Vector2& _position) {
	setWorldPos(_position);
	transform.position+=collider->forceOut(MAPMASK|ENEMYMASK);
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
	sceneCam->update();
	sceneCam->use();
}
#pragma endregion// Player

#pragma region Enemy
void Enemy::on_loop(const double& delta) {
	Vector2 targetPos=Pathfinder::WorldToGrid(target->getWorldPos());
	if(lastSpottedPos!=targetPos) {
		Vector2 worldPos=getWorldPos();
		BoxCollider::CollitionData hit=collider->raycast(worldPos, target->getWorldPos(), MAPMASK);
		if(!((bool)hit)) {
			//double startTime=glfwGetTime();
			path=pathfinder->pathfind(Pathfinder::WorldToGrid(worldPos), targetPos);
			//Log("Final Time: "+std::to_string((glfwGetTime()-startTime)*1000.0)+"ms");
			lastSpottedPos=targetPos;
		}
	}

	float travelDist=enemySpeed*static_cast<float>(delta)*mapScale*(1+spacing);
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
	iconRenderer->setWorldPos(gridToMinimap(WorldToGrid(getWorldPos())));
}
Enemy::Enemy(const Vector3& enemyModulate, Texture* enemyTex, Pathfinder* _pathfinder, hasTransform2D* _target, const Vector2& _position) :
	Object(), hasTransform2D(_position, 0.0f, Vector2(mapScale), Vector2::Center, 0.0f), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), pathfinder(_pathfinder), target(_target) {
	renderer=spriteRenderer->addSprite(Vector4(enemyModulate, 1.0f), enemyTex, Vector2::ZERO, 2.0f, Vector2(playerSize));
	addChild(renderer);
	collider=new BoxCollider(ENEMYMASK, false, Vector2::ZERO, 100.0f, playerHitbox);
	renderer->addChild(&collider->transform);
	iconRenderer=uiHandler->Sprite(Vector4(enemyModulate, 0.75f), enemyTex, gridToMinimap(WorldToGrid(getWorldPos())), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), Vector2::Center);
	engine->sub_loop(this);
}
#pragma endregion// Enemy

#pragma region Instance
void Instance::on_click(const Vector2& pos) {
	if(broken&&(player->getWorldPos()-pos).sqrMagnitude()<=(std::pow(flashlightRadius*mapScale/2.0f, 2)))
		fixInstance();
}
Instance::Instance(ClickDetector* clickDetector, Texture* _instanceUnlitTex, Texture* _instanceWorkingTex, Texture* _instanceBrokenTex, const Vector2& _position, const Vector2& _anchor, const float& _rotAngle) :
	Clickable(clickDetector), hasTransform2D(_position, 0.0f, Vector2(mapScale), _anchor, _rotAngle), instanceWorkingTex(_instanceWorkingTex), instanceBrokenTex(_instanceBrokenTex) {
	engine_assert(_instanceUnlitTex!=nullptr, "[Instance]: Instance unlit texture is nullptr");
	engine_assert(instanceWorkingTex!=nullptr, "[Instance]: Instance working texture is nullptr");
	engine_assert(instanceBrokenTex!=nullptr, "[Instance]: Instance broken texture is nullptr");
	broken=static_cast<float>(std::rand())/static_cast<float>(RAND_MAX)<=(instanceBrokenChance/100.0f);
	addChild(staticSpriteRenderer->addSprite(Vector4(0.5f, 0.5f, 0.5f, 1.0f), _instanceUnlitTex, Vector2::ZERO, 1.0f));
	stateQuad=instanceStateSpritesRenderer->addSprite(Vector4::ONE, broken ? instanceBrokenTex : instanceWorkingTex, Vector2::ZERO, 2.0f);
	addChild(stateQuad);
	addChild(new BoxCollider(MAPMASK, true, Vector2::ZERO, 100.0f));
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
	assert(parsedMap, "Map data failed to parse.");
	// setup Engine
	engine=new Engine(HD1080P, "Ghost Game", VSYNC);
	assert(parsedMap, "Engine failed to init.");
	//not required but I do this
	engine_assert(TextRenderer::initCharacterMap(), "Error initializing font.");
	// setup cameras
	cam=new OrthoCam(Vector2::ZERO, viewRange);
	uiCam=new OrthoCam(viewRange, viewRange*2.0f);
	// setup textures
	Texture* playerTex=new Texture(playerTexPath);
	Texture* enemyTex=new Texture(enemyTexPath);

	Texture* backgroundTex=new Texture(mapTexPath);
	Texture* minimapTex=new Texture(minimapTexPath);

	Texture* instanceUnlitTex=new Texture(instanceUnlitTexPath);
	Texture* instanceWorkingTex=new Texture(instanceWorkingTexPath);
	Texture* instanceBrokenTex=new Texture(instanceBrokenTexPath);
	minimapSize=Vector2(static_cast<float>(minimapTex->width), static_cast<float>(minimapTex->height))*minimapScale;
#pragma endregion// Engine setup
	// setup renderers
	spriteRenderer=new BatchedSpriteRenderer(cam);
	staticSpriteRenderer=new StaticBatchedSpriteRenderer(cam);
	instanceStateSpritesRenderer=new StaticBatchedSpriteRenderer(cam);
	uiHandler=new UiHandler(uiCam);
	ColliderDebugLineRenderer=new BatchedLineRenderer(cam, 3.0f);
	StaticColliderDebugLineRenderer=new StaticBatchedLineRenderer(cam, 3.0f);
	instanceClickDetector=new ClickDetector(cam);
	// player object
	player=new Player(cam, playerModulate, playerTex, flashlightColor, GridToWorld(playerOffset));
	finder=std::make_unique<Pathfinder>();
	enemy=new Enemy(enemyModulate, enemyTex, finder.get(), player, GridToWorld(playerOffset-Vector2(0.0f, 2.0f)));
#pragma region Map setup
	staticSpriteRenderer->addSprite(Vector4::ZERO, backgroundTex, Vector2::ZERO, 0.0f, fullMapSize, Vector2::BottomLeft);// map background
	//ColliderDebug=true;// makes hitboxes visible
	for(const std::array<int, 5>&dat:instanceData) {
		Vector2 pos=GridToWorld(Vector2(static_cast<float>(dat[0]), static_cast<float>(dat[1]))+Vector2(0.5f, 0.5f));
		new Instance(instanceClickDetector, instanceUnlitTex, instanceWorkingTex, instanceBrokenTex, pos, Vector2::Center, 0.0f);
	}
	for(const Vector3& line:horizontalWallData) {// horizontal walls
		new BoxCollider(MAPMASK, true, GridToWorld(Vector2((line.z+line.y)/2.0f, line.x)), 100.0f, Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*3.0f), spacing*3.0f)*mapScale);
	}
	for(const Vector3& line:verticalWallData) {// vertical walls
		new BoxCollider(MAPMASK, true, GridToWorld(Vector2(line.x, (line.z+line.y)/2.0f)), 100.0f, Vector2(spacing*3.0f, ((line.z-line.y)*(1.0f+spacing)+spacing*3.0f))*mapScale);
	}
#pragma endregion// Map setup
	// setup UI
	uiHandler->Sprite(Vector4(Vector3::ONE, 0.75f), minimapTex, Vector2(0.0f, viewRange.y*2.0f), 0.0f, minimapSize, Vector2::TopLeft);// minimap

	/*Button* testButton=uiHandler->createButton(
		Vector4(0.125f, 0.125f, 0.125f, 1.0f), Vector4(0.1f, 0.1f, 0.1f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f),
		Vector2(viewRange.x*2.0f, 0.0f), 0.0f, Vector2(25.0f, 25.0f), Vector2::BottomRight);
	testButton->addChild(uiHandler->Text("X", Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(6.0f, 4.0f), 15.0f, 2.0f, Vector2::BottomRight));
	testButton->onrelease=[&]() { engine->Close(); };

	TextInput* testInput=uiHandler->createTextInput("", "Type here.",
		Vector2(viewRange.x*2.0f-25.0f, 0.0f), 0.0f, Vector2(400.0f, 25.0f), Vector2::BottomRight);
	testInput->onenter=[&](std::string str) { Log(str); };*/

	fpsText=uiHandler->Text("Fps Avg:,high:,low:", Vector4(0.75f, 0.25f, 0.25f, 1.0f), viewRange*2.0f+Vector2(-120.0f, -1.0f), 15.0f, 2.0f, Vector2::TopLeft);
#ifdef _DEBUG
	debugText=uiHandler->Text("Pos:\nTime:\nMemory:\nDraw calls:", Vector4(0.75f, 0.75f, 0.75f, 1.0f), Vector2(1.0f, 1.0f), 15.0f, 2.0f, Vector2::BottomLeft);
#endif// _DEBUG
	staticSpriteRenderer->bind();
	instanceStateSpritesRenderer->bind();
	StaticColliderDebugLineRenderer->bind();

	Shader* tmp=createSpriteSheetShader(new Texture("Resources/spritesheet.png"), Vector4::ONE);
	cam->bindShader(tmp);
	cam->use();
	Vector2i atlasSize=Vector2i(13, 10);
	Vector2i uvPos=Vector2i(5, 0);
	Vector2i uvSize=Vector2i::ONE;
	test=new SpritesheetAnimationRenderer(tmp, atlasSize, 3, 0.125, uvPos, uvSize, 0.0f, GridToWorld(playerOffset), 15.0f, Vector2(mapScale));

	// run main loop
	engine->renderLoop=Render;
	engine->Start();
	delete engine;
	colliders.clear();
	return 1;
}
void Render(const double& delta) {
	fpsText->text="Fps:"+std::to_string(engine->fpsAvg);
#ifdef _DEBUG
	// set debug text
	debugText->text="Pos:"+std::to_string(player->getWorldPos())+"\nTime:"+std::to_string(glfwGetTime())+"\nMemory:"+std::to_string(allocatedPeak)+" bytes\nDraw calls:"+std::to_string(engine->drawCalls);
	allocatedPeak=0;
#endif// _DEBUG
	// draw scene
	spriteRenderer->draw();
	staticSpriteRenderer->draw();
	test->draw();
	player->flashlightStencilOn();
	instanceStateSpritesRenderer->draw();
	player->flashlightStencilOff();
	//debug stuff
	if(enemy->debugRen) enemy->debugRen->draw();
	if(ColliderDebug) { ColliderDebugLineRenderer->draw();StaticColliderDebugLineRenderer->draw(); }
	uiHandler->draw();// draws ui
}

int main(int argc, char** argv) {
	int value=Run();
#ifdef _DEBUG
	Log("Press enter to close . . .");
	std::cin.get();
#endif// _DEBUG
	return value;
}