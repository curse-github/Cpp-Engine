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
Shader* createTexShader(Texture* tex, const Vector4& modulate);
Shader* createTextShader();
Shader* createBatchedShader(const std::vector<Texture*>& textures);

struct PathfinderData;
class Pathfinder {
	protected:
	static unsigned int GridToIndex(const Vector2& grid) { return (unsigned int)(grid.x*mapSize.y*2+grid.y); }
	static unsigned int MaxIndex() { return (unsigned int)(mapSize.x*mapSize.y)*4; }
	friend PathfinderData;

	bool isWall(const Vector2& pos);
	bool isValid(const Vector2& pos);
	float calcH(const Vector2& a, const Vector2& b);
	bool isValidMove(const Vector2& a, const Vector2& dir);
	std::array<Vector2, 8> movements={
		Vector2(-1.0f, -1.0f),
		Vector2(0.0f, -1.0f),
		Vector2(1.0f, -1.0f),
		Vector2(1.0f, 0.0f),
		Vector2(1.0f, 1.0f),
		Vector2(0.0f, 1.0f),
		Vector2(-1.0f, 1.0f),
		Vector2(-1.0f, 0.0f)
	};
	std::array<std::vector<Vector2>, 8> obstructions={ {
		{ Vector2(0.0f, -1.0f), Vector2(-1.0f, 0.0f) },
		{},
		{ Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{},
		{ Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f) },
		{},
		{ Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f) },
		{}
		} };
	std::vector<bool> wallMap;
	public:
	static Vector2 GridToWorld(const Vector2& grid) { return Vector2(grid.x/2.0f*mapScale*(1.0f+spacing), grid.y/2.0f*mapScale*(1.0f+spacing)); }
	static Vector2 WorldToGrid(const Vector2& world) { return Vector2(std::round(world.x/mapScale/(1.0f+spacing)*2.0f), std::round(world.y/mapScale/(1.0f+spacing)*2.0f)); }
	Pathfinder();
	std::vector<Vector2> pathfind(const Vector2& a, const Vector2& b);
	bool allowDiagonals=true;
};
class FpsTracker : Object {
	protected:
	double lastFrames[500]={};
	int avgFps=0;
	int highFps=0;
	int lowFps=0;
	float frameTime=0;
	void on_loop(const double& delta) override;
	public:
	FpsTracker();
	int getAvgFps();
	int getHighFps();
	int getLowFps();
	float getFrameTime();
};

extern bool ColliderDebug;
class BoxCollider : public LineRenderer {
	public:
	typedef short unsigned int maskType;
	maskType mask;
	BoxCollider() : LineRenderer(), mask(0) { initialized=false; }
	BoxCollider(Shader* _debugLineShader, maskType _mask, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE);
	struct RaycastHit {
		bool hit;
		Vector2 point;
		float dist;
		RaycastHit() : hit(false), dist(FLT_MAX), point(Vector2()) {};
		RaycastHit(const Vector2& _point, const float& _dist) : hit(true), point(_point), dist(_dist) {};
		operator bool() const { return hit; };
		RaycastHit operator||(const RaycastHit& b) const {
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
			switch(((int)a->hit)+((int)raycast.hit)*2) {
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