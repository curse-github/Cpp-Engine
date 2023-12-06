#pragma once
#ifndef _MAINH
#define _MAINH
#include "Lib.h"
#include "Engine.h"
#include "Json.h"
#include <math.h>
#define PI 3.14159265
#define TAU 6.2831853

const float minimapScale=0.15f;
Vector2 minimapSize;

Vector2 gridToWorld(Vector2 grid) {
	return Vector2(grid.x*mapScale*(1.0f+spacing), (mapSize.y-grid.y)*mapScale*(1.0f+spacing));
}
Vector2 worldToGrid(Vector2 world) {
	return Vector2(world.x/mapScale/(1.0f+spacing), mapSize.y-world.y/mapScale/(1.0f+spacing));
}
Vector2 gridToMinimap(Vector2 grid) {
	return Vector2(grid.x/mapSize.x*minimapSize.x, 540-grid.y/mapSize.y*minimapSize.y);
}

class FpsTracker;
class BoxCollider;
class Player;
class Enemy;

Engine* engine;
FpsTracker* tracker;
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

std::vector<BoxCollider*> instanceColliders;

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
	void on_loop(double delta) override;
	void resolveCollitions();
	public:
	Vector2 position;
	Enemy() : Object(), position(Vector2()), renderer(nullptr), collider(nullptr), iconRenderer(nullptr) {}
	Enemy(Engine* _engine, Vector2 _position, Shader* enemyShader, Shader* iconShader);
	void setPos(Vector2 pos);
};

int main(int argc, char** argv);
void Loop(double delta);
#endif// MAINH