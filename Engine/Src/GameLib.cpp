#include "GameLib.h"

const float minimapScale=0.15f;
Vector2 minimapSize;

Vector2 gridToMinimap(const Vector2& grid) { return Vector2(grid.x/mapSize.x*minimapSize.x, 540+(grid.y/mapSize.y-1)*minimapSize.y); }
Vector2 WorldToGrid(const Vector2& world) { return Vector2(world.x/mapScale/(1.0f+spacing), world.y/mapScale/(1.0f+spacing)); }
Vector2 GridToWorld(const Vector2& grid) { return Vector2(grid.x*mapScale*(1.0f+spacing), grid.y*mapScale*(1.0f+spacing)); }

#pragma region Pathfinder
template <typename T, unsigned int N> bool arryHas(std::array<T, N> arry, T value) { for(T aryValue : arry) if(aryValue==value) return true; return false; }
template <typename T, unsigned int N> unsigned int arryFind(std::array<T, N> arry, T value) { for(unsigned int i=0; i<static_cast<unsigned int>((sizeof arry)/(sizeof value)); i++) if(arry[i]==value) return static_cast<int>(i); return -1; }
template <typename T> unsigned int vectorFind(std::vector<T> vec, T value) { for(unsigned int i=0; i<vec.size(); i++) if(vec[i]==value) return static_cast<int>(i); return -1; }

Pathfinder::Pathfinder() {
	wallMap.resize(MaxIndex());
	wallMap.assign(wallMap.size(), false);
	for(const std::array<int, 5>&data:instanceData) {
		for(unsigned short int x=0; x<3; x++) {
			for(unsigned short int y=0; y<3; y++) {
				wallMap[Pathfinder::GridToIndex(Vector2usi(data[0]*2u+x, data[1]*2u+y))]=true;
			}
		}
	}
	for(const Vector3& line:horizontalWallData) {
		for(unsigned short int x=static_cast<unsigned short int>(line.y)*2u; x<static_cast<unsigned short int>(line.z)*2u+1u; x++) {
			wallMap[Pathfinder::GridToIndex(Vector2usi(x, static_cast<unsigned short int>(line.x)*2u))]=true;
		}
	}
	for(const Vector3& line:verticalWallData) {
		for(unsigned short int y=static_cast<unsigned short int>(line.y)*2u; y<static_cast<unsigned short int>(line.z)*2u+1u; y++) {
			wallMap[Pathfinder::GridToIndex(Vector2usi(static_cast<unsigned short int>(line.x)*2u, y))]=true;
		}
	}
}
bool Pathfinder::isWall(const Vector2usi& pos) {
	if(pos.x<0||pos.y<0||pos.x>(mapSize.x*2-1)||pos.y>(mapSize.y*2-1)) return true;// dont bother checking if its out of range;
	return wallMap[Pathfinder::GridToIndex(pos)]==true;
}
bool Pathfinder::isValid(const Vector2usi& pos) {
	return !isWall(pos);
}
float Pathfinder::calcH(const Vector2usi& a, const Vector2usi& b) {
	Vector2si vec=(static_cast<Vector2si>(a)-static_cast<Vector2si>(b)).abs();
	if(allowDiagonals) return std::min(vec.x, vec.y)*sqrtf(2)+abs(vec.x-vec.y);
	else return static_cast<float>(vec.x+vec.y);
}
bool Pathfinder::isValidMove(const Vector2usi& pos, const Vector2si& dir) {
	unsigned int index=arryFind(movements, dir);
	if(index==-1) return false;
	std::vector<Vector2si> obstructionsLst=obstructions[index];
	for(Vector2usi obstruction : obstructionsLst) if(!isValid(pos+obstruction)) return false;
	return true;
}
std::vector<Vector2> Pathfinder::pathfind(const Vector2usi& A, const Vector2usi& B) {
	if(A==B||!isValid(A)||!isValid(B)||isWall(B)) return {};
	unsigned int maxKey=Pathfinder::MaxIndex();
	std::vector<unsigned int> openIndices;
	if(dataMap.size()!=maxKey) dataMap.resize(maxKey);
	for(unsigned int i=0; i<maxKey; i++) dataMap[i]=PathfinderData();
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
		vec2<unsigned short> lowestPos=lowestData->Pos;
		// if reached the goal, return the path to get to there from point A
		if(lowestPos==B) {
			std::vector<Vector2> path={ (Vector2)Pathfinder::GridToWorld(lowestData->Pos) };
			PathfinderData data=dataMap[lowestData->FromKey];
			while(data.FromKey!=-1) {
				path.push_back((Vector2)Pathfinder::GridToWorld(data.Pos));
				data=dataMap[data.FromKey];
			}
			path.shrink_to_fit();
			return path;
		}
		// evaluate squares around the currentSquare for their F cost
		for(Vector2si movement : movements) {
			if(!isValidMove(lowestPos, movement)) continue;
			Vector2usi pos=lowestPos+movement;
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
Shader* createColorShader(const Vector4& color) {
	Shader* shader=new Shader("Shaders/basic.vert", "Shaders/color.frag");
	shader->setFloat4("color", color);
	return shader;
}
Shader* createTexShader(Texture* tex, const Vector4& modulate) {
	Shader* shader=new Shader("Shaders/basic.vert", "Shaders/tex.frag");
	shader->setTexture("_texture", tex, 0);
	shader->setFloat4("modulate", modulate);
	return shader;
}
Shader* createTextShader() {
	Shader* shader=new Shader("Shaders/basic.vert", "Shaders/text.frag");
	return shader;
}
Shader* createDotColorShader(const Vector4& color) {
	Shader* shader=new Shader("Shaders/basic.vert", "Shaders/dotColor.frag");
	shader->setFloat4("color", color);
	return shader;
}
Shader* createDotTexShader(Texture* tex, const Vector4& modulate) {
	Shader* shader=new Shader("Shaders/basic.vert", "Shaders/dotTex.frag");
	shader->setTexture("_texture", tex, 0);
	shader->setFloat4("modulate", modulate);
	return shader;
}
Shader* createSpriteSheetShader(Texture* tex, const Vector4& modulate) {
	Shader* shader=new Shader("Shaders/spritesheet.vert", "Shaders/tex.frag");
	shader->setTexture("_texture", tex, 0);
	shader->setFloat4("modulate", modulate);
	return shader;
}
#pragma endregion// Shader creators
#pragma region BoxCollider
std::vector<BoxCollider*> colliders;
bool ColliderDebug=false;
BatchedLineRenderer* ColliderDebugLineRenderer;
StaticBatchedLineRenderer* StaticColliderDebugLineRenderer;
BoxCollider::BoxCollider(maskType _mask, const bool& _isStatic, const Vector2& _position, const float& _zIndex, const Vector2& _scale) :
	hasTransform2D(_position, _zIndex, _scale, Vector2::Center, 0.0f),
	mask(_mask), isStatic(_isStatic) {
	engine_assert((Engine::instance!=nullptr)&&!Engine::instance->ended, "[BoxCollider]: Engine not avaliable.");
	if(isStatic) addChild(StaticColliderDebugLineRenderer->addRect(Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector2::ZERO, _zIndex));// make a static collider outline
	else addChild(ColliderDebugLineRenderer->addRect(Vector4(0.0f, 0.0f, 1.0f, 1.0f), Vector2::ZERO, _zIndex));// make a non-static collider outline
	colliders.push_back(this);
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
void BoxCollider::LineBoxCollide(const Vector2& p1, const Vector2& p2, const Vector2& boxPos, const Vector2& boxSize, CollitionData* b) {
	float boundingRadius=(boxSize/2.0f).length();
	// check if theres honestly any chance of collision
	if(((p1-boxPos).length()-boundingRadius)>(p1-p2).length()) return;
	Vector2 halfScale=boxSize/2.0f;
	// check if p1 is inside the box
	if((boxPos.x-p1.x<=halfScale.x)&&(p1.x-boxPos.x<=halfScale.x)&&(boxPos.y-p1.y<=halfScale.y)&&(p1.y-boxPos.y<=halfScale.y)) { b->hit=false; return; }
	// check if the line has hit any of the rectangle's sides
	CollitionData::combine(b, BoxCollider::lineLineIntersection(p1, p2, boxPos-halfScale, boxPos+Vector2(-halfScale.x, halfScale.y)), Vector2(-1.0f, 0.0f));
	CollitionData::combine(b, BoxCollider::lineLineIntersection(p1, p2, boxPos+Vector2(halfScale.x, -halfScale.y), boxPos+halfScale), Vector2(1.0f, 0.0f));
	CollitionData::combine(b, BoxCollider::lineLineIntersection(p1, p2, boxPos-halfScale, boxPos+Vector2(halfScale.x, -halfScale.y)), Vector2(0.0f, -1.0f));
	CollitionData::combine(b, BoxCollider::lineLineIntersection(p1, p2, boxPos+Vector2(-halfScale.x, halfScale.y), boxPos+halfScale), Vector2(0.0f, 1.0f));
}
BoxCollider::CollitionData BoxCollider::detectCollision(const BoxCollider* other) {
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
			return CollitionData(RaycastHit(Vector2(), std::min(collisionX1, collisionX2)), vec.normalized());
		} else {
			Vector2 vec(0.0f, collisionY2-collisionY1);
			return CollitionData(RaycastHit(Vector2(), std::min(collisionY1, collisionY2)), vec.normalized());
		}
	} else {
		return CollitionData();
	}
}
void BoxCollider::sweepDetectCollision(const BoxCollider* other, const Vector2& vec, CollitionData* b) {
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	Vector2 otherWorldPos=other->getWorldPos();
	Vector2 otherWorldScale=other->getWorldScale();
	float boundingRadius=(worldScale/2.0f).length();
	float otherBoundingRadius=(otherWorldScale/2.0f).length();
	LineBoxCollide(otherWorldPos, otherWorldPos+vec, worldPos, worldScale+otherWorldScale, b);
}
void BoxCollider::LineCollide(const Vector2& p1, const Vector2& p2, CollitionData* b) {
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	LineBoxCollide(p1, p2, worldPos, worldScale, b);
}

Vector2 BoxCollider::forceOut(const maskType& collisionMask) {
	Vector2 oldPos=getWorldPos();
	Vector2 newPos=oldPos;
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		BoxCollider::CollitionData collition=collider->detectCollision(this);
		newPos+=collition.normal*collition.dist;
		setWorldPos(newPos);
	}
	Vector2 vec=newPos-oldPos;
	setWorldPos(oldPos);
	return vec;
}
Vector2 BoxCollider::tryMove(const Vector2& tryVec, const maskType& collisionMask) {
	Vector2 oldPos=getWorldPos();
	BoxCollider::CollitionData* hit=new BoxCollider::CollitionData();
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		collider->sweepDetectCollision(this, tryVec, hit);
	}
	if(!hit->hit) { delete hit; return tryVec; }
	Vector2 vec=(hit->normal*tryVec.dot(hit->normal));
	Vector2 newVec=tryVec-vec;
	delete hit;
	if(newVec.x==0.0f&&newVec.y==0.0f) { return newVec; }// newVec is (0,0)
	hit=new BoxCollider::CollitionData();
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		collider->sweepDetectCollision(this, newVec, hit);
	}
	if(!hit->hit) { delete hit; return newVec; } else { delete hit; return Vector2::ZERO; }
}
BoxCollider::CollitionData BoxCollider::raycast(const Vector2& p1, const Vector2& p2, const maskType& collisionMask) const {
	BoxCollider::CollitionData out=BoxCollider::CollitionData();
	for(BoxCollider* collider:colliders) {
		if((collider->mask&collisionMask)==0) continue;
		collider->LineCollide(p1, p2, &out);
	}
	return out;
}
#pragma endregion
// BoxCollider