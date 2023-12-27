#include "GameLib.h"

const float minimapScale=0.15f;
Vector2 minimapSize;

Vector2 gridToMinimap(const Vector2& grid) { return Vector2(grid.x/mapSize.x*minimapSize.x, 540+(grid.y/mapSize.y-1)*minimapSize.y); }
Vector2 WorldToGrid(const Vector2& world) { return Vector2(world.x/mapScale/(1.0f+spacing), world.y/mapScale/(1.0f+spacing)); }
Vector2 GridToWorld(const Vector2& grid) { return Vector2(grid.x*mapScale*(1.0f+spacing), grid.y*mapScale*(1.0f+spacing)); }

#pragma region Pathfinder
template <typename T, unsigned int N> bool arryHas(std::array<T, N> arry, T value) { for(T aryValue : arry) if(aryValue==value) return true; return false; }
template <typename T, unsigned int N> unsigned int arryFind(std::array<T, N> arry, T value) { for(unsigned int i=0; i<((unsigned int)((sizeof arry)/(sizeof value))); i++) if(arry[i]==value) return (int)i; return -1; }
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
bool Pathfinder::isWall(const Vector2& pos) {
	if(pos.x<0||pos.y<0||pos.x>(mapSize.x*2-1)||pos.y>(mapSize.y*2-1)) return true;// dont bother checking if its out of range;
	return wallMap[Pathfinder::GridToIndex(pos)]==true;
}
bool Pathfinder::isValid(const Vector2& pos) {
	return !isWall(pos);
}
float Pathfinder::calcH(const Vector2& a, const Vector2& b) {
	Vector2 vec=(a-b).abs();
	if(allowDiagonals) return std::min(vec.x, vec.y)*sqrtf(2)+abs(vec.x-vec.y);
	else return vec.x+vec.y;
}
bool Pathfinder::isValidMove(const Vector2& pos, const Vector2& dir) {
	unsigned int index=arryFind(movements, dir);
	if(index==-1) return false;
	std::vector<Vector2> obstructionsLst=obstructions[index];
	for(Vector2 obstruction : obstructionsLst) if(!isValid(pos+obstruction)) return false;
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
std::vector<Vector2> Pathfinder::pathfind(const Vector2& A, const Vector2& B) {
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

#pragma region Shader creators
Shader* createColorShader(Engine* engine, const Vector4& color) {
	Shader* shader=new Shader(engine, "Shaders/basic.vert", "Shaders/color.frag");
	if(!shader->initialized) return shader;
	shader->setFloat4("color", color);
	return shader;
}
Shader* createTexShader(Engine* engine, Texture* tex, const Vector4& modulate) {
	Shader* shader=new Shader(engine, "Shaders/basic.vert", "Shaders/tex.frag");
	if(!shader->initialized) return shader;
	shader->setTexture("_texture", tex, 0);
	shader->setFloat4("modulate", modulate);
	return shader;
}
Shader* createTextShader(Engine* engine) {
	Shader* shader=new Shader(engine, "Shaders/basic.vert", "Shaders/text.frag");
	return shader;
}
Shader* createBatchedShader(Engine* engine, const std::vector<Texture*>& textures) {
	Shader* shader=new Shader(engine, "Shaders/batch.vert", "Shaders/texBatch.frag");
	if(!shader->initialized) return shader;
	shader->setTextureArray("_textures");
	for(unsigned int i=0; i<textures.size(); i++) shader->setTexture("_", textures[i], (int)i);
	return shader;
}
#pragma endregion// Shader creators

#pragma region FpsTracker
void FpsTracker::on_loop(const double& delta) {
	if(engine->ended||!initialized) return;
	for(unsigned int i=1; i<500; i++) { lastFrames[i-1]=lastFrames[i]; }// move values back
	lastFrames[499]=delta;// put delta at the end
	double sum=0.0f;
	for(unsigned int i=0; i<500; i++) { sum+=lastFrames[i]; }// sum values
	avgFps=(int)(500.0f/sum+0.5);
	frameTime=(float)sum;//1000.0f*1000.0f;

	highFps=0;
	lowFps=100000;
	for(unsigned int i=0; i<500; i++) {
		if((1/lastFrames[i])>highFps) highFps=(int)((1/lastFrames[i])+0.5);
		if((1/lastFrames[i])<lowFps) lowFps=(int)((1/lastFrames[i])+0.5);
	}
}
FpsTracker::FpsTracker(Engine* _engine) : Object(_engine) {
	if(!initialized) return;
	engine->sub_loop(this);
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
bool ColliderDebug=false;
std::vector<BoxCollider*> colliders;
BoxCollider::BoxCollider(Engine* _engine, Vector2 _position, Vector2 _scale, maskType _mask, Shader* _debugLineShader) :
	LineRenderer(_engine, _debugLineShader, { Vector2(-_scale.x/2, _scale.y/2), Vector2(_scale.x/2, _scale.y/2), Vector2(_scale.x/2, -_scale.y/2), Vector2(-_scale.x/2, -_scale.y/2) }, 3.0f, _position, true),
	mask(_mask) {
	if(!initialized) return;
	colliders.push_back(this);
}
BoxCollider::RaycastHit::operator bool() const { return hit; }
BoxCollider::RaycastHit BoxCollider::RaycastHit::operator||(const BoxCollider::RaycastHit& b) const {
	RaycastHit out;
	switch(((int)hit)+((int)b.hit)*2) {
		case 0://neither
			out=RaycastHit();
			break;
		case 1://a&&!b
			out=RaycastHit(*this);
			break;
		case 2://b&&!a
			out=b;
			break;
		case 3://a&&b
			if(dist<=b.dist) out=RaycastHit(*this);// return whichever is closer
			else out=b;
			break;
	}
	return out;
}
BoxCollider::CollitionData::operator bool() const { return hit; }
BoxCollider::CollitionData BoxCollider::CollitionData::operator||(const BoxCollider::CollitionData& b) const {
	CollitionData out;
	switch(((int)hit)+((int)b.hit)*2) {
		case 0://neither
			out=CollitionData();
			break;
		case 1://a&&!b
			out=CollitionData(*this);
			break;
		case 2://b&&!a
			out=b;
			break;
		case 3://a&&b
			if(dist<=b.dist) out=CollitionData(*this);// return whichever is closer
			else out=b;
			break;
	}
	return out;
}
BoxCollider::RaycastHit BoxCollider::lineLineIntersection(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) {
	// calculate the direction of the lines
	float den=(p4.y-p3.y)*(p2.x-p1.x)-(p4.x-p3.x)*(p2.y-p1.y);
	float t=((p4.x-p3.x)*(p1.y-p3.y)-(p4.y-p3.y)*(p1.x-p3.x))/den;
	float u=((p2.x-p1.x)*(p1.y-p3.y)-(p2.y-p1.y)*(p1.x-p3.x))/den;
	// if uA and uB are between 0-1, lines are intersecting
	if(t>=0&&t<=1&&u>=0&&u<=1) {
		return RaycastHit(p1+(p2-p1)*t, t);
	}
	return RaycastHit();
}
BoxCollider::CollitionData BoxCollider::LineBoxCollide(const Vector2& p1, const Vector2& p2, const Vector2& boxPos, const Vector2& boxSize) {
	float boundingRadius=(boxSize/2.0f).length();
	//check if theres honestly any chance of collision
	if(((p1-boxPos).length()-boundingRadius)>(p1-p2).length()) return CollitionData();
	//draw();
	// check if the line has hit any of the rectangle's sides
	CollitionData collision=CollitionData();
	Vector2 halfScale=boxSize/2.0f;
	collision=collision||CollitionData(BoxCollider::lineLineIntersection(p1, p2, boxPos-halfScale, boxPos+Vector2(-halfScale.x, halfScale.y)), Vector2(-1.0f, 0.0f));
	collision=collision||CollitionData(BoxCollider::lineLineIntersection(p1, p2, boxPos+Vector2(halfScale.x, -halfScale.y), boxPos+halfScale), Vector2(1.0f, 0.0f));
	collision=collision||CollitionData(BoxCollider::lineLineIntersection(p1, p2, boxPos-halfScale, boxPos+Vector2(halfScale.x, -halfScale.y)), Vector2(0.0f, -1.0f));
	collision=collision||CollitionData(BoxCollider::lineLineIntersection(p1, p2, boxPos+Vector2(-halfScale.x, halfScale.y), boxPos+halfScale), Vector2(0.0f, 1.0f));
	// if ANY of the above are true, the line has hit the rectangle
	return collision;
}
BoxCollider::CollitionData BoxCollider::detectCollision(BoxCollider* other) {
	if(engine->ended||!initialized) return BoxCollider::CollitionData();
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	Vector2 otherWorldPos=other->getWorldPos();
	Vector2 otherWorldScale=other->getWorldScale();
	float boundingRadius=(worldScale/2.0f).length();
	float otherBoundingRadius=(otherWorldScale/2.0f).length();
	if(((worldPos-otherWorldPos).length()-(boundingRadius+otherBoundingRadius))>=0) return CollitionData();
	if(worldPos==otherWorldPos) return CollitionData();
	// collision x-axis?
	float collisionX1=((worldPos.x+worldScale.x/2)-(otherWorldPos.x-otherWorldScale.x/2));
	float collisionX2=((otherWorldPos.x+otherWorldScale.x/2)-(worldPos.x-worldScale.x/2));
	// collision y-axis?
	float collisionY1=((worldPos.y+worldScale.y/2)-(otherWorldPos.y-otherWorldScale.y/2));
	float collisionY2=((otherWorldPos.y+otherWorldScale.y/2)-(worldPos.y-worldScale.y/2));
	// collision only if on both axes
	if(collisionX1>0&&collisionX2>0&&collisionY1>0&&collisionY2>0) {
		if(std::abs(collisionX2-collisionX1)/worldScale.x>std::abs(collisionY2-collisionY1)/worldScale.y) {
			Vector2 vec(collisionX2-collisionX1, 0.0f);
			return CollitionData(Vector2(), vec.normalized(), std::min(collisionX1, collisionX2));
		} else {
			Vector2 vec(0.0f, collisionY2-collisionY1);
			return CollitionData(Vector2(), vec.normalized(), std::min(collisionY1, collisionY2));
		}
	} else {
		return CollitionData();
	}
}
BoxCollider::CollitionData BoxCollider::sweepDetectCollision(BoxCollider* other, const Vector2& vec) {
	if(engine->ended||!initialized) return BoxCollider::CollitionData();
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	Vector2 otherWorldPos=other->getWorldPos();
	Vector2 otherWorldScale=other->getWorldScale();
	float boundingRadius=(worldScale/2.0f).length();
	float otherBoundingRadius=(otherWorldScale/2.0f).length();
	if(((worldPos-otherWorldPos).length()-(boundingRadius+otherBoundingRadius))-vec.length()>=0) return CollitionData();
	//draw();
	return LineBoxCollide(otherWorldPos, otherWorldPos+vec, worldPos, worldScale+otherWorldScale);
}
BoxCollider::CollitionData BoxCollider::LineCollide(const Vector2& p1, const Vector2& p2) {
	if(engine->ended||!initialized) return BoxCollider::CollitionData();
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	return LineBoxCollide(p1, p2, worldPos, worldScale);
}

Vector2 BoxCollider::forceOut(const maskType& collisionMask) {
	if(engine->ended||!initialized) return Vector2::ZERO;
	Vector2 oldPos=position;
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		BoxCollider::CollitionData collition=collider->detectCollision(this);
		position+=collition.normal*collition.dist;
	}
	Vector2 vec=oldPos-position;
	position=oldPos;
	return vec;
}
Vector2 BoxCollider::tryMove(const Vector2& tryVec, const maskType& collisionMask) {
	if(engine->ended||!initialized) return Vector2::ZERO;
	Vector2 oldPos=position;
	BoxCollider::CollitionData hit=BoxCollider::CollitionData();
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		hit=hit||(collider->sweepDetectCollision(this, tryVec));
	}
	//if (!hit.hit) return tryVec; else return Vector2::ZERO;
	if(hit.hit) {
		Vector2 newVec=tryVec-(hit.normal*tryVec.dot(hit.normal));
		if(newVec.x!=0.0f||newVec.y!=0.0f) {
			hit=BoxCollider::CollitionData();
			for(BoxCollider* collider:colliders) {
				if((collider->mask&mask)==0) continue;
				hit=hit||(collider->sweepDetectCollision(this, newVec));
			}
			if(!hit.hit) return newVec;
			else return Vector2::ZERO;
		}
	} else return tryVec;
	Vector2 vec=oldPos-position;
	position=oldPos;
	return vec;
}
BoxCollider::CollitionData BoxCollider::raycast(const Vector2& p1, const Vector2& p2, const maskType& collisionMask) {
	if(engine->ended||!initialized) return BoxCollider::CollitionData();
	BoxCollider::CollitionData out=BoxCollider::CollitionData();
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		BoxCollider::CollitionData hit=collider->LineCollide(p1, p2);
		out=out||hit;
	}
	return out;
}
#pragma endregion// BoxCollider