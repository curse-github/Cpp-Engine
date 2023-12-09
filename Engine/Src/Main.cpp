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
	size(_size), boundingRadius(sqrt(size.x* size.x+size.y*size.y)) {}
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
	colliders.push_back(collider);
	flashlightRenderer=new SpriteRenderer(engine, flashlightShader, position, flashlightRange*mapScale, 1.0f);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(WorldToGrid(position)), Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), 1.0f);
	uiRenderers.push_back(iconRenderer);

	resolveCollitions();
	renderer->position=position;
	flashlightRenderer->position=position;
	sceneCam->position=position;
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();

	engine->sub_key(this);
	engine->sub_loop(this);
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
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
void Player::resolveCollitions() {
	if(engine->ended||!initialized) return;
	//if (inputs[4]) return;// noclip when left shift
	collider->position=position;
	for(unsigned int i=0; i<colliders.size(); i++) {
		CollitionData collition=colliders[i]->checkCollision(collider);
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
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
	sceneCam->update();
	sceneCam->use();
}
#pragma endregion// Player

#pragma region Pathfinder
template <typename T, std::size_t N> bool arryHas(std::array<T, N> arry, T value) { for(T aryValue : arry) if(aryValue==value) return true; return false; }
template <typename T> unsigned int vectorFind(std::vector<T> vec, T value) { for(unsigned int i=0; i<vec.size(); i++) if(vec[i]==value) return (int)i; return -1; }

Pathfinder::Pathfinder() {
	wallMap.resize(MaxIndex());
	wallMap.assign(wallMap.size(), false);
	for(const std::array<int, 5>&data:instanceData) {
		for(float x=0; x<3; x++) {
			for(float y=0; y<3; y++) {
				wallMap[Pathfinder::GridToIndex(Vector2(((float)data[0])*2.0f+x, ((float)data[1])*2.0f+y))]=true;
			}
		}
	}
	for(const Vector3& line:horizontalWallData) {
		for(float x=line.y*2.0f; x<line.z*2.0f+1; x++) {
			wallMap[Pathfinder::GridToIndex(Vector2(x, line.x*2.0f))]=true;
		}
	}
	for(const Vector3& line:verticalWallData) {
		for(float y=line.y*2.0f; y<line.z*2.0f+1; y++) {
			wallMap[Pathfinder::GridToIndex(Vector2(line.x*2.0f, y))]=true;
		}
	}
}
bool Pathfinder::isWall(Vector2 pos) {
	if(pos.x<0||pos.y<0||pos.x>(mapSize.x*2-1)||pos.y>(mapSize.y*2-1)) return true;// dont bother checking if its out of range;
	return wallMap[Pathfinder::GridToIndex(pos)]==true;
}
bool Pathfinder::isValid(Vector2 pos) {
	return !isWall(pos);
}
float Pathfinder::calcH(Vector2 a, Vector2 b) {
	Vector2 vec=(a-b).abs();
	if(allowDiagonals) return std::min(vec.x, vec.y)*sqrtf(2)+abs(vec.x-vec.y);
	else return vec.x+vec.y;
}
bool Pathfinder::isValidMove(Vector2 pos, Vector2 dir) {
	if(!arryHas(movements, dir)) return false;
	for(unsigned int i=0; i<((unsigned int)((sizeof obstructions)/(sizeof obstructions[0]))); i++) {
		std::tuple<Vector2, std::vector<Vector2>> obstructionObj=obstructions[i];
		if(dir!=std::get<0>(obstructionObj)) continue;
		std::vector<Vector2> obstructionsLst=std::get<1>(obstructionObj);
		for(Vector2 obstruction : obstructionsLst) if(!isValid(pos+obstruction)) return false;
	}
	return true;
}
struct PathfinderData {
	Vector2 Pos;
	unsigned int FromKey;
	float G;
	float F;
	bool open;
	bool initialized;
	PathfinderData() :
		Pos(Vector2()), FromKey(Pathfinder::MaxIndex()), G(FLT_MAX), F(FLT_MAX), open(false), initialized(false) {}
	PathfinderData(Vector2 _Pos, unsigned int _FromKey, float _G, float _F, bool _open) :
		Pos(_Pos), FromKey(_FromKey), G(_G), F(_F), open(_open), initialized(true) {}
};
std::vector<Vector2> Pathfinder::pathfind(Vector2 A, Vector2 B) {
	if(A==B||!isValid(A)||!isValid(B)||isWall(B)) return {};
	unsigned int maxKey=Pathfinder::MaxIndex();
	std::vector<PathfinderData> dataMap;
	std::vector<unsigned int> openIndices;
	dataMap.resize(maxKey);
	unsigned int tmp=Pathfinder::GridToIndex(A);
	dataMap[tmp]=PathfinderData(A, -1, 0, calcH(A, B), true);
	openIndices.push_back(tmp);
	std::vector<unsigned int> successorKeys;
	while(true) {
		// find the square with the lowest F cost
		unsigned int lowestKey=maxKey;
		//double testTime=glfwGetTime();
		if(successorKeys.size()>0) {
			lowestKey=successorKeys.back();
			successorKeys.pop_back();
		} else {
			float lowestF=-1.0f;
			for(unsigned int i=0; i<openIndices.size(); i++) {
				unsigned int key=openIndices[i];
				PathfinderData data=dataMap[key];
				if(data.open) {//if open
					float F=data.F;
					if((F<lowestF)||(lowestKey==maxKey)) {
						lowestKey=key;
						lowestF=F;
						successorKeys.clear();
					} else if(F==lowestF) {
						successorKeys.push_back(key);
					}
				}
			}
		}
		//Log("Test Time: "+std::to_string((glfwGetTime()-testTime)*1000.0)+"ms");
		// return if there are no more unevaluated positions
		if(lowestKey==maxKey) return {};
		// close current position
		PathfinderData* lowestData=&dataMap[lowestKey];
		lowestData->open=false;
		openIndices.erase(openIndices.begin()+vectorFind(openIndices, lowestKey));
		Vector2 lowestPos=lowestData->Pos;
		// if reached the goal, return the path to get to there from point A
		if(lowestPos==B) {
			std::vector<Vector2> path={ Pathfinder::GridToWorld(lowestData->Pos) };
			PathfinderData data=dataMap[lowestData->FromKey];
			while(data.FromKey!=-1) {
				path.push_back(Pathfinder::GridToWorld(data.Pos));
				data=dataMap[data.FromKey];
			}
			path.shrink_to_fit();
			return path;
		}
		// evaluate squares around the currentSquare for their F cost
		for(Vector2 movement : movements) {
			if(!isValidMove(lowestPos, movement)) continue;
			Vector2 pos=lowestPos+movement;
			if(!isValid(pos)) continue;
			unsigned int key=Pathfinder::GridToIndex(pos);
			PathfinderData* oldData=&dataMap[key];
			if(oldData->initialized&&!oldData->open) continue;
			float G=lowestData->G+movement.length();
			PathfinderData newData=PathfinderData(pos, lowestKey, G, G+calcH(pos, B), true);
			if(newData.F<oldData->F) {
				openIndices.push_back(key);
				*oldData=newData;
			}
		}
	}
	return {};
}
#pragma endregion// Pathfinder

#pragma region Enemy
Enemy::Enemy(Engine* _engine, Vector2 _position, Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, Player* _target)
	: Object(_engine), position(_position), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(_lineShader), pathfinder(_pathfinder), target(_target) {
	if(!initialized) return;

	renderer=new SpriteRenderer(engine, enemyShader, position, playerSize*mapScale, 1);
	sceneRenderers.push_back(renderer);
	collider=new BoxCollider(engine, position, playerHitbox*playerSize*mapScale, lineShader);
	colliders.push_back(collider);
	iconRenderer=new SpriteRenderer(engine, iconShader, gridToMinimap(WorldToGrid(position)), Vector2(minimapSize.x/mapSize.x, minimapSize.y/mapSize.y), 1.0f);
	uiRenderers.push_back(iconRenderer);

	resolveCollitions();
	renderer->position=position;
	iconRenderer->position=gridToMinimap(WorldToGrid(position));

	engine->sub_key(this);
	engine->sub_loop(this);
}
void Enemy::updateDebugLine() {
	delete debugRen;
	if(path.size()>0) {
		std::vector<Vector2> line;
		for(const Vector2& pos:path) line.push_back(pos);
		line.push_back(position);
		debugRen=new LineRenderer(engine, lineShader, line, 3, false);
	} else debugRen=nullptr;
}
void Enemy::on_loop(double delta) {
	if(engine->ended||!initialized) return;
	Vector2 targetPos=Pathfinder::WorldToGrid(target->position);
	if(targetLastPos!=targetPos) {
		//double startTime=glfwGetTime();
		path=pathfinder->pathfind(Pathfinder::WorldToGrid(position), targetPos);
		//Log("Final Time: "+std::to_string((glfwGetTime()-startTime)*1000.0)+"ms");
		targetLastPos=targetPos;
	}
	float travelDist=enemySpeed*((float)delta)*mapScale*(1+spacing);
	while(path.size()>0) {
		if((position-target->position).length()<=mapScale*(1+spacing)) { path.clear();break; }
		Vector2 dir=path.back()-position;
		float dist=dir.length();
		if(travelDist==dist) {
			position+=dir;
			path.pop_back();
			break;
		} else if(travelDist<dist) {
			position+=dir.normalized()*travelDist;
			break;
		} else {// travelDist>dist
			position+=dir;
			travelDist-=dist;
			path.pop_back();
			continue;
		}
	}
	updateDebugLine();
	renderer->position=position;
	collider->position=position;
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
}
void Enemy::resolveCollitions() {
	if(engine->ended||!initialized) return;
	collider->position=position;
	for(unsigned int i=0; i<colliders.size(); i++) {
		CollitionData collition=colliders[i]->checkCollision(collider);
		collider->position+=collition.normal*collition.dist;
	}
	position=collider->position;
}
void Enemy::setPos(Vector2 _position) {
	if(engine->ended||!initialized) return;
	position=_position;
	resolveCollitions();
	renderer->position=position;
	iconRenderer->position=gridToMinimap(WorldToGrid(position));
}
#pragma endregion// Enemy

int main(int argc, char** argv) {
#pragma region setup

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
	cam=new OrthoCam(engine, Vector2(), viewRange);
	uiCam=new OrthoCam(engine, Vector2(480.0f, 270.0f), Vector2(960.0f, 540.0f));
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
		!instanceBrokenTex->initialized
		) {
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

	backgroundShader=createTexShader(backgroundTex, Vector4());
	minimapShader=createTexShader(minimapTex, Vector4(1.0f, 1.0f, 1.0f, 0.75f));

	instanceUnlitShader=createTexShader(instanceUnlitTex, Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	instanceWorkingShader=createTexShader(instanceWorkingTex, Vector4());
	instanceBrokenShader=createTexShader(instanceBrokenTex, Vector4());

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
#pragma endregion// setup

	// player object
	player=new Player(engine, cam, GridToWorld(playerOffset), playerShader, flashlightShader, playerIconShader);
	enemy=new Enemy(engine, GridToWorld(playerOffset+Vector2(0.0f, 2.0f)), enemyShader, enemyIconShader, lineShader, finder.get(), player);
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
	for(const std::array<int, 5>&dat:instanceData) {
		Vector2 pos=GridToWorld(Vector2(((float)dat[0]+0.5f), ((float)dat[1]+0.5f)));
		sceneRenderers.push_back(new SpriteRenderer(engine, instanceUnlitShader, pos, Vector2(mapScale, mapScale), 2.0f));
		bool broken=((float)std::rand())/((float)RAND_MAX)<=(instanceBrokenChance/100.0f);
		instanceStateRenderers.push_back(new SpriteRenderer(engine, broken ? instanceBrokenShader : instanceWorkingShader, pos, Vector2(mapScale, mapScale), 3.0f));
		colliders.push_back(new BoxCollider(engine, pos, Vector2(mapScale, mapScale), lineShader));
	}
	// create horizontal wall colliders
	for(const Vector3& line:horizontalWallData) {
		colliders.push_back(new BoxCollider(engine, GridToWorld(Vector2((line.z+line.y)/2, line.x)), Vector2(((line.z-line.y)*(1.0f+spacing)+spacing*3)*mapScale, spacing*3*mapScale), lineShader));
	}
	// create vertical wall colliders
	for(const Vector3& line:verticalWallData) {
		colliders.push_back(new BoxCollider(engine, GridToWorld(Vector2(line.x, (line.z+line.y)/2)), Vector2(spacing*3*mapScale, ((line.z-line.y)*(1.0f+spacing)+spacing*3)*mapScale), lineShader));
	}
	// run main loop
	engine->renderLoop=Loop;
	Log("Engine initialized successfully.");
	engine->Loop();
	//destruction
	delete engine;
	sceneRenderers.clear();
	instanceStateRenderers.clear();
	uiRenderers.clear();
	debugText.clear();
	colliders.clear();
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
	if(enemy->debugRen) enemy->debugRen->draw();

	for(BoxCollider* col:colliders) if(col->shouldDraw(playerPos, viewRange)) col->draw();
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* ren:uiRenderers) ren->draw();
	for(TextRenderer* ren:debugText) ren->draw();
}