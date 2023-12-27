#include "Main.h"
#include <map>

#pragma region Player
void Player::on_key(GLFWwindow* window, const int& key, const int& scancode, const int& action, const int& mods) {
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
	position+=collider->tryMove(inputVec, MAPMASK|ENEMYMASK);
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
Player::Player(Engine* _engine, OrthoCam* _sceneCam, const Vector2& _position, Shader* playerShader, Shader* flashlightShader, Shader* iconShader) :
	Object(_engine), Transform2D(_position, 0.0f, Vector2(mapScale), Vector2::Center, 0.0f), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(_sceneCam), flashlightRenderer(nullptr), iconRenderer(nullptr) {
	if(!initialized) return;
	addChild(sceneCam);
	sceneCam->position=Vector2::ZERO;
	renderer=new SpriteRenderer(_engine, playerShader, Vector2::ZERO, 1.0f, Vector2(playerSize), Vector2::Center);
	sceneRenderers.push_back(renderer);
	addChild(renderer);
	collider=new BoxCollider(_engine, Vector2::ZERO, playerHitbox, PLAYERMASK, lineShader);
	renderer->addChild(collider);
	flashlightRenderer=new SpriteRenderer(engine, flashlightShader, Vector2::ZERO, 1.0f, flashlightRange, Vector2::Center);
	addChild(flashlightRenderer);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(WorldToGrid(position)), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), Vector2::Center);
	uiRenderers.push_back(iconRenderer);

	position+=collider->forceOut(MAPMASK|ENEMYMASK);
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();

	engine->sub_key(this);
	engine->sub_loop(this);
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
void Player::setPos(const Vector2& _position) {
	if(engine->ended||!initialized) return;
	position=_position;
	position+=collider->forceOut(MAPMASK|ENEMYMASK);
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
#pragma endregion// Player

#pragma region Enemy
void Enemy::setDebugLine(std::vector<Vector2> line) {
	if(engine->ended||!initialized) return;
	delete debugRen;
	std::vector<Vector2> renderLine;
	for(const Vector2& pos:line) renderLine.push_back(pos);
	renderLine.push_back(position);
	if(line.size()>0) debugRen=new LineRenderer(engine, lineShader, renderLine, 3, false);
	else debugRen=nullptr;
}
void Enemy::on_loop(const double& delta) {
	if(engine->ended||!initialized) return;
	Vector2 targetPos=Pathfinder::WorldToGrid(target->position);
	if(lastSpottedPos!=targetPos) {
		BoxCollider::CollitionData hit=collider->raycast(position, target->position, MAPMASK);
		if(!((bool)hit)) {
			//double startTime=glfwGetTime();
			path=pathfinder->pathfind(Pathfinder::WorldToGrid(position), targetPos);
			//Log("Final Time: "+std::to_string((glfwGetTime()-startTime)*1000.0)+"ms");
			lastSpottedPos=targetPos;
		}// else setDebugLine({ position, hit.point });
	}

	float travelDist=enemySpeed*((float)delta)*mapScale*(1+spacing);
	while(path.size()>0&&travelDist>0) {
		if((position-target->position).length()<=mapScale*(1+spacing)) { path.clear();break; }
		Vector2 dir=path.back()-position;
		float dist=dir.length();
		if(travelDist<dist) {
			position+=dir.normalized()*travelDist;
			break;
		} else {// travelDist>dist
			position+=dir;
			travelDist-=dist;
			path.pop_back();
			continue;
		}
	}
	//setDebugLine(path);
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
}
Enemy::Enemy(Engine* _engine, const Vector2& _position, Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, Player* _target) :
	Object(_engine), Transform2D(_position, 0.0f, Vector2(mapScale), Vector2::Center, 0.0f), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(_lineShader), pathfinder(_pathfinder), target(_target) {
	if(!initialized) return;
	renderer=new SpriteRenderer(engine, enemyShader, Vector2::ZERO, 1.0f, Vector2(playerSize), Vector2::Center);
	sceneRenderers.push_back(renderer);
	addChild(renderer);
	collider=new BoxCollider(engine, Vector2::ZERO, playerHitbox, ENEMYMASK, lineShader);
	renderer->addChild(collider);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(WorldToGrid(position)), 1.0f, Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), Vector2::Center);
	uiRenderers.push_back(iconRenderer);

	engine->sub_loop(this);
}
#pragma endregion// Enemy

int Run() {
#pragma region Setup
	// load map data
	loadMapData("map");
	if(!parsedMap) {
		Log("Map data failed to load.");
		return 0;
	}
	// setup engine
	engine=new Engine(HD1080P, "Ghost Game", false);
	if(!engine->initialized||engine->ended) {
		Log("Engine failed to init.");
		return 0;
	}
	// setup other stuff
	tracker=new FpsTracker(engine);
	finder=std::make_unique<Pathfinder>();
	// setup cameras
	cam=new OrthoCam(engine, Vector2::ZERO, viewRange);
	uiCam=new OrthoCam(engine, viewRange, viewRange*2.0f);
	if(engine->ended||!cam->initialized||!uiCam->initialized) {
		Log("Cameras failed to init.");
		engine->Delete();
		return 0;
	}
	// setup textures
	Texture* playerTex=new Texture(engine, playerTexPath);
	Texture* flashlightTex=new Texture(engine, flashlightTexPath);

	Texture* enemyTex=new Texture(engine, enemyTexPath);

	Texture* backgroundTex=new Texture(engine, mapTexPath);
	Texture* minimapTex=new Texture(engine, minimapTexPath);

	Texture* instanceUnlitTex=new Texture(engine, instanceUnlitTexPath);
	Texture* instanceWorkingTex=new Texture(engine, instanceWorkingTexPath);
	Texture* instanceBrokenTex=new Texture(engine, instanceBrokenTexPath);
	if(engine->ended||!playerTex->initialized||!flashlightTex->initialized||
		!backgroundTex->initialized||!minimapTex->initialized||
		!instanceUnlitTex->initialized||!instanceWorkingTex->initialized||
		!instanceBrokenTex->initialized) {
		Log("Textures failed to init.");
		engine->Delete();
		return 0;
	}
	minimapSize=Vector2((float)minimapTex->width, (float)minimapTex->height)*minimapScale;
	// setup shaders
	playerShader=createTexShader(engine, playerTex, Vector4(playerModulate, 1.0f));
	flashlightShader=createTexShader(engine, flashlightTex, Vector4(flashlightColor, 0.25f));
	playerIconShader=createTexShader(engine, playerTex, Vector4(playerModulate, 0.75f));

	enemyShader=createTexShader(engine, enemyTex, Vector4(enemyModulate, 1.0f));
	enemyIconShader=createTexShader(engine, enemyTex, Vector4(enemyModulate, 0.75f));

	backgroundShader=createTexShader(engine, backgroundTex, Vector4::ZERO);
	minimapShader=createTexShader(engine, minimapTex, Vector4(1.0f, 1.0f, 1.0f, 0.75f));

	instanceShader=createBatchedShader(engine, { instanceUnlitTex });
	instanceStateShader=createBatchedShader(engine, { instanceWorkingTex, instanceBrokenTex });

	lineShader=createColorShader(engine, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	if(engine->ended||
		!playerShader->initialized||!flashlightShader->initialized||
		!playerIconShader->initialized||!backgroundShader->initialized||
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
	player=new Player(engine, cam, GridToWorld(playerOffset), playerShader, flashlightShader, playerIconShader);
	enemy=new Enemy(engine, GridToWorld(playerOffset-Vector2(0.0f, 2.0f)), enemyShader, enemyIconShader, lineShader, finder.get(), player);
	// map and minimap
	sceneRenderers.push_back(new SpriteRenderer(engine, backgroundShader, Vector2::ZERO, 0.0f, fullMapSize, Vector2::BottomLeft));// background
	uiRenderers.push_back(new SpriteRenderer(engine, minimapShader, Vector2(0.0f, HD1080P.y/2.0f), 0.0f, minimapSize, Vector2::TopLeft));// minimap
	// create instances
	//ColliderDebug=true;// make hitboxes visible
	instanceRenderer=new StaticBatchedSpriteRenderer(engine, instanceShader);
	instanceStateRenderer=new StaticBatchedSpriteRenderer(engine, instanceStateShader);
	for(const std::array<int, 5>&dat:instanceData) {
		Vector2 pos=GridToWorld(Vector2((float)dat[0], (float)dat[1])+Vector2(0.5f, 0.5f));
		bool broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
		instanceRenderer->addQuad(pos, 2.0f, Vector2(mapScale), Vector4(0.5f, 0.5f, 0.5f, 1.0f), 0.0f);
		instanceStateRenderer->addQuad(pos, 3.0f, Vector2(mapScale), Vector4::ONE, broken ? 1.0f : 0.0f);
		//if(broken) Log(pos);
		new BoxCollider(engine, pos, Vector2(mapScale), MAPMASK, lineShader);
	}
	instanceRenderer->bind();
	instanceStateRenderer->bind();
	for(const Vector3& line:horizontalWallData) {// horizontal walls
		new BoxCollider(engine, GridToWorld(Vector2((line.z+line.y)/2.0f, line.x)), Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*3.0f)*mapScale, spacing*3.0f*mapScale), MAPMASK, lineShader);
	}
	for(const Vector3& line:verticalWallData) {// vertical walls
		new BoxCollider(engine, GridToWorld(Vector2(line.x, (line.z+line.y)/2.0f)), Vector2(spacing*3.0f, ((line.z-line.y)*(1.0f+spacing)+spacing*3.0f))*mapScale, MAPMASK, lineShader);
	}
	// setup text renderers
	textRenderer=new BatchedTextRenderer(engine, uiCam);
	fpsText=textRenderer->addText("Fps Avg:,high:,low:", Vector4(0.75f, 0.25f, 0.25f, 1.0f), viewRange+Vector2(-232.0f, -1.0f), 15.0f, 2.0f, Vector2::TopLeft);
#ifdef _DEBUG
	debugText=textRenderer->addText("Pos:\nTime:", Vector4(0.75f, 0.75f, 0.75f, 1.0f), Vector2(1.0f, 1.0f), 15.0f, 2.0f, Vector2::BottomLeft);
#endif// _DEBUG
	if(engine->ended||!TextRenderer::characterMapInitialized) {
		Log("Fonts failed to init.");
		engine->Delete();
		return 0;
	}
	// run main loop
	engine->renderLoop=Loop;
	Log("Engine initialized successfully.");
	engine->Loop();
	//destruction
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
	debugText->text="Pos: "+player->position.to_string()+"\nTime: "+std::to_string(glfwGetTime());
#endif// _DEBUG

	// draw scene
	for(Renderer2D* ren:sceneRenderers) if(ren->shouldDraw(cam)) ren->draw();
	instanceRenderer->draw();
	player->flashlightStencilOn();
	instanceStateRenderer->draw();
	player->flashlightStencilOff();
	if(enemy->debugRen) enemy->debugRen->draw();
	if(ColliderDebug) {
		for(BoxCollider* col:colliders) if(col->shouldDraw(cam)) col->draw();
		player->collider->draw();
	}
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* ren:uiRenderers) ren->draw();
	textRenderer->draw();
}

int main(int argc, char** argv) {
	int value=Run();
#ifdef _DEBUG
	Log("Press enter to close . . .");
	std::cin.get();
#endif// _DEBUG
	return value;
}