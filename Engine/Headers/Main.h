#pragma once
#ifndef _MAINH
#define _MAINH
#include "Lib.h"
#include "Engine.h"
#include "Json.h"
#include <math.h>
#include <array>
#define PI 3.14159265
#define TAU 6.2831853

const float minimapScale=0.15f;
Vector2 minimapSize;

Vector2 gridToMinimap(Vector2 grid) { return Vector2(grid.x/mapSize.x*minimapSize.x, 540+(grid.y/mapSize.y-1)*minimapSize.y); }
Vector2 WorldToGrid(Vector2 world) { return Vector2(world.x/mapScale/(1.0f+spacing), world.y/mapScale/(1.0f+spacing)); }
Vector2 GridToWorld(Vector2 grid) { return Vector2(grid.x*mapScale*(1.0f+spacing), grid.y*mapScale*(1.0f+spacing)); }

struct PathfinderData;
class Pathfinder {
	protected:
	static unsigned int GridToIndex(Vector2 grid) { return (unsigned int)(grid.x*mapSize.y*2+grid.y); }
	static unsigned int MaxIndex() { return (unsigned int)(mapSize.x*mapSize.y)*4; }
	friend PathfinderData;

	bool isWall(Vector2 pos);
	bool isValid(Vector2 pos);
	float calcH(Vector2 a, Vector2 b);
	bool isValidMove(Vector2 a, Vector2 dir);
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
	std::array<std::tuple<Vector2, std::vector<Vector2>>, 4> obstructions={ {
		{ Vector2(-1.0f, -1.0f), { Vector2(0.0f, -1.0f), Vector2(-1.0f, 0.0f) } },
		{ Vector2(1.0f, -1.0f), { Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f) } },
		{ Vector2(1.0f, 1.0f), { Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f) } },
		{ Vector2(-1.0f, 1.0f), { Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f) } }
		} };
	std::vector<bool> wallMap;
	public:
	static Vector2 GridToWorld(Vector2 grid) { return Vector2(grid.x/2.0f*mapScale*(1.0f+spacing), grid.y/2.0f*mapScale*(1.0f+spacing)); }
	static Vector2 WorldToGrid(Vector2 world) { return Vector2(std::round(world.x/mapScale/(1.0f+spacing)*2.0f), std::round(world.y/mapScale/(1.0f+spacing)*2.0f)); }
	Pathfinder();
	std::vector<Vector2> pathfind(Vector2 a, Vector2 b);
	bool allowDiagonals=true;
};

class FpsTracker;
class BoxCollider;
class Player;
class Enemy;

Engine* engine;
FpsTracker* tracker;
std::unique_ptr<Pathfinder> finder;
Player* player;
Enemy* enemy;

OrthoCam* cam;
OrthoCam* uiCam;

Shader* playerShader;
Shader* flashlightShader;
Shader* playerIconShader;

Shader* enemyShader;
Shader* enemyIconShader;

Shader* backgroundShader;
Shader* minimapShader;

Shader* instanceUnlitShader;
Shader* instanceWorkingShader;
Shader* instanceBrokenShader;

Shader* lineShader;
Shader* textShader;

std::vector<Renderer2D*> sceneRenderers;
std::vector<Renderer2D*> instanceStateRenderers;
std::vector<Renderer*> uiRenderers;
std::vector<TextRenderer*> debugText;

std::vector<BoxCollider*> colliders;

Shader* createTexShader(Texture* tex, Vector4 modulate);
Shader* createColorShader(Vector4 color);
Shader* createTextShader();

class FpsTracker : Object {
	protected:
	double lastFrames[60]={};
	int avgFps=0;
	int highFps=0;
	int lowFps=0;
	float frameTime=0;
	void on_loop(double delta) override;
	public:
	FpsTracker() : Object() {}
	FpsTracker(Engine* _engine);
	int getAvgFps();
	int getHighFps();
	int getLowFps();
	float getFrameTime();
};

bool ColliderDebug=false;
struct CollitionData {
	Vector2 normal;
	float dist;
	CollitionData(Vector2 _normal, float _dist) : normal(_normal), dist(_dist) {}
};
class BoxCollider : public LineRenderer {
	public:
	Vector2 size;
	float boundingRadius;
	BoxCollider() : LineRenderer(), size(Vector2(0)), boundingRadius(0.0f) {}
	BoxCollider(Engine* _engine, Vector2 _pos, Vector2 _size, Shader* _debugLineShader);
	void draw();
	CollitionData checkCollision(BoxCollider* other);
};

class Player : public Object {
	protected:
	StencilSimple flashlightStencil;
	OrthoCam* sceneCam;
	SpriteRenderer* renderer;
	BoxCollider* collider;
	SpriteRenderer* flashlightRenderer;
	SpriteRenderer* iconRenderer;
	int inputs[5]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void on_loop(double delta) override;
	void resolveCollitions();
	public:
	Vector2 position;
	Player() : Object(), position(Vector2()), flashlightStencil(StencilSimple()), sceneCam(nullptr), renderer(nullptr), collider(nullptr), flashlightRenderer(nullptr), iconRenderer(nullptr) {}
	Player(Engine* _engine, OrthoCam* _sceneCam, Vector2 _position, Shader* playerShader, Shader* flashlightShader, Shader* iconShader);
	void flashlightStencilOn();
	void flashlightStencilOff();
	void setPos(Vector2 pos);
};

class Enemy : public Object {
	protected:
	SpriteRenderer* renderer;
	BoxCollider* collider;
	SpriteRenderer* iconRenderer;
	Shader* lineShader;
	Pathfinder* pathfinder;
	Player* target;
	Vector2 targetLastPos;
	std::vector<Vector2> path;
	void updateDebugLine();
	void on_loop(double delta) override;
	void resolveCollitions();
	public:
	Vector2 position;
	LineRenderer* debugRen=nullptr;
	Enemy() : Object(), position(Vector2()), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(nullptr), pathfinder(nullptr), target(nullptr) {}
	Enemy(Engine* _engine, Vector2 _position, Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, Player* _target);
	void setPos(Vector2 pos);
};

Vector2 HD1080P(1920.0, 1080.0);
Vector2 viewRange(480.0f, 270.0f);
int main(int argc, char** argv);
void Loop(double delta);
#endif// MAINH