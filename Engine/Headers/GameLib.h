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

Shader* createTexShader(Engine* engine, Texture* tex, const Vector4& modulate);
Shader* createColorShader(Engine* engine, const Vector4& color);
Shader* createTextShader(Engine* engine);
Shader* createBatchedShader(Engine* engine, const std::vector<Texture*>& textures);

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
	FpsTracker() : Object() {}
	FpsTracker(Engine* _engine);
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
	BoxCollider() : LineRenderer(), mask(0) {}
	BoxCollider(Engine* _engine, Vector2 _position, Vector2 _scale, maskType _mask, Shader* _debugLineShader);
	struct RaycastHit {
		bool hit;
		Vector2 point;
		float dist;
		RaycastHit() : hit(false), dist(FLT_MAX), point(Vector2()) {};
		RaycastHit(const Vector2& _point, const float& _dist) : hit(true), point(_point), dist(_dist) {};
		operator bool() const;
		RaycastHit operator||(const RaycastHit& b) const;
	};
	struct CollitionData {
		bool hit;
		Vector2 point;
		Vector2 normal;
		float dist;
		CollitionData() : hit(false), dist(FLT_MAX), point(Vector2()) {};
		CollitionData(const Vector2& _point, const Vector2& _normal, const float& _dist) : hit(true), point(_point), normal(_normal), dist(_dist) {};
		CollitionData(const RaycastHit& raycast, const Vector2& _normal) : hit(raycast.hit), point(raycast.point), normal(_normal), dist(raycast.dist) {};
		operator bool() const;
		CollitionData operator||(const CollitionData& b) const;
	};
	static RaycastHit lineLineIntersection(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4);
	static CollitionData LineBoxCollide(const Vector2& p1, const Vector2& p2, const Vector2& boxPos, const Vector2& boxSize);
	CollitionData detectCollision(BoxCollider* other);
	CollitionData sweepDetectCollision(BoxCollider* other, const Vector2& vec);
	CollitionData LineCollide(const Vector2& p1, const Vector2& p2);

	Vector2 forceOut(const maskType& collisionMask);
	Vector2 tryMove(const Vector2& tryVec, const maskType& collisionMask);
	BoxCollider::CollitionData raycast(const Vector2& p1, const Vector2& p2, const maskType& collisionMask);
};
extern std::vector<BoxCollider*> colliders;
const BoxCollider::maskType PLAYERMASK=1;
const BoxCollider::maskType MAPMASK=2;
const BoxCollider::maskType ENEMYMASK=4;

#endif// _GAME_LIB_H