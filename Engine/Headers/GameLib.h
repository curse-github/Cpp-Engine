#pragma once
#ifndef _GAME_LIB_H
#define _GAME_LIB_H

#include "Json.h"
#include "Engine.h"
#include "BatchedRenderers.h"

extern const float minimapScale;
extern Vector2 minimapSize;

Vector2 gridToMinimap(const Vector2& grid);
Vector2 WorldToGrid(const Vector2& world);
Vector2 GridToWorld(const Vector2& grid);

Shader* createColorShader(const Vector4& color);
Shader* createTexShader(Texture* tex, const Vector4& modulate=Vector4::ZERO);
Shader* createTextShader();
Shader* createDotColorShader(const Vector4& color);
Shader* createDotTexShader(Texture* tex, const Vector4& modulate=Vector4::ZERO);

class Pathfinder {
	typedef vec2<unsigned short int> Vector2usi;
	typedef vec2<short int> Vector2si;
	protected:
	static unsigned int GridToIndex(const Vector2usi& grid) { return static_cast<unsigned int>(grid.x*mapSize.y*2+grid.y); }
	static unsigned int MaxIndex() { return static_cast<unsigned int>(mapSize.x*mapSize.y)*4; }
	struct PathfinderData {
		Vector2usi Pos;
		unsigned int FromKey;
		float G;
		float F;
		bool open;
		bool initialized;
		PathfinderData() :
			Pos(Vector2usi()), FromKey(Pathfinder::MaxIndex()), G(FLT_MAX), F(FLT_MAX), open(false), initialized(false) {}
		PathfinderData(const Vector2usi& _Pos, const unsigned int& _FromKey, const float& _G, const float& _F, const bool& _open) :
			Pos(_Pos), FromKey(_FromKey), G(_G), F(_F), open(_open), initialized(true) {}
	};
	std::vector<PathfinderData> dataMap;

	bool isWall(const Vector2usi& pos);
	bool isValid(const Vector2usi& pos);
	float calcH(const Vector2usi& a, const Vector2usi& b);
	bool isValidMove(const Vector2usi& a, const Vector2si& dir);
	std::array<Vector2si, 8> movements={
		Vector2si(-1, -1),
		Vector2si(0, -1),
		Vector2si(1, -1),
		Vector2si(1, 0),
		Vector2si(1, 1),
		Vector2si(0, 1),
		Vector2si(-1, 1),
		Vector2si(-1, 0)
	};
	std::array<std::vector<Vector2si>, 8> obstructions={ {
		{ Vector2si(0, -1), Vector2si(-1, 0) },
		{},
		{ Vector2si(0, -1), Vector2si(1, 0) },
		{},
		{ Vector2si(0, 1), Vector2si(1, 0) },
		{},
		{ Vector2si(0, 1), Vector2si(-1, 0) },
		{}
		} };
	std::vector<bool> wallMap;
	public:
	static inline Vector2 GridToWorld(const Vector2usi& grid) { return Vector2(grid.x/2.0f*mapScale*(1.0f+spacing), grid.y/2.0f*mapScale*(1.0f+spacing)); }
	static inline Vector2usi WorldToGrid(const Vector2& world) { return Vector2usi(static_cast<unsigned short int>(std::round(world.x/mapScale/(1.0f+spacing)*2.0f)), static_cast<unsigned short int>(std::round(world.y/mapScale/(1.0f+spacing)*2.0f))); }
	Pathfinder();
	std::vector<Vector2> pathfind(const Vector2usi& a, const Vector2usi& b);
	bool allowDiagonals=true;
};

extern bool ColliderDebug;
extern BatchedLineRenderer* ColliderDebugLineRenderer;
extern StaticBatchedLineRenderer* StaticColliderDebugLineRenderer;
class BoxCollider : public hasTransform2D {
	public:
	bool initialized=false;
	typedef short unsigned int maskType;
	maskType mask;
	bool isStatic=false;
	BoxCollider() : hasTransform2D(), mask(0) { initialized=false; }
	BoxCollider(maskType _mask, const bool& _isStatic=false, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	struct RaycastHit {
		bool hit;
		Vector2 point;
		float dist;
		RaycastHit() : hit(false), dist(FLT_MAX), point(Vector2()) {};
		RaycastHit(const Vector2& _point, const float& _dist) : hit(true), point(_point), dist(_dist) {};
		operator bool() const { return hit; };
		RaycastHit operator||(const RaycastHit& b) const {
			RaycastHit out;
			switch(static_cast<int>(hit)+static_cast<int>(b.hit)*2) {
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
		};
	};
	struct CollitionData {
		bool hit;
		Vector2 point;
		Vector2 normal;
		float dist;
		CollitionData() : hit(false), dist(FLT_MAX), point(Vector2()) {};
		CollitionData(const RaycastHit& raycast, const Vector2& _normal) : hit(raycast.hit), point(raycast.point), normal(_normal), dist(raycast.dist) {};
		static void combine(CollitionData* a, const RaycastHit& raycast, const Vector2& _normal) {
			switch(static_cast<int>(a->hit)+static_cast<int>(raycast.hit)*2) {
				case 0://neither
					break;
				case 1://a&&!b
					break;
				case 2://b&&!a
					a->hit=raycast.hit;
					a->point=raycast.point;
					a->normal=_normal;
					a->dist=raycast.dist;
					break;
				case 3://a&&b
					if(raycast.dist<a->dist) {// if a is closer than new data
						a->hit=raycast.hit;
						a->point=raycast.point;
						a->normal=_normal;
						a->dist=raycast.dist;
					}
					break;
			}
		}
		operator bool() const { return hit; }
		CollitionData operator||(const CollitionData& b) const {
			CollitionData out;
			switch(static_cast<int>(hit)+static_cast<int>(b.hit)*2) {
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
	};
	static RaycastHit lineLineIntersection(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4);
	static void LineBoxCollide(const Vector2& p1, const Vector2& p2, const Vector2& boxPos, const Vector2& boxSize, CollitionData* b=new CollitionData());
	BoxCollider::CollitionData detectCollision(const BoxCollider* other);
	void sweepDetectCollision(const BoxCollider* other, const Vector2& vec, CollitionData* b=new CollitionData());
	void LineCollide(const Vector2& p1, const Vector2& p2, CollitionData* b=new CollitionData());

	Vector2 forceOut(const maskType& collisionMask);
	Vector2 tryMove(const Vector2& tryVec, const maskType& collisionMask);
	BoxCollider::CollitionData raycast(const Vector2& p1, const Vector2& p2, const maskType& collisionMask) const;
};
extern std::vector<BoxCollider*> colliders;
const BoxCollider::maskType PLAYERMASK=1;
const BoxCollider::maskType MAPMASK=2;
const BoxCollider::maskType ENEMYMASK=4;

#endif// _GAME_LIB_H