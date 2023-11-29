#pragma once
#ifndef _MAINH
#define _MAINH
#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"
#include "Json.h"
#include <math.h>
#define PI 3.14159265
#define TAU 6.2831853

const float minimapScale=0.15f;
Vector2 minimapSize;

Vector2 gridToWorld(Vector2 grid) {
	return Vector2(grid.x*mapScale*(1.0+spacing), (mapSize.y-grid.y)*mapScale*(1.0+spacing));
}
Vector2 worldToGrid(Vector2 world) {
	return Vector2(world.x/mapScale/(1.0+spacing), mapSize.y-world.y/mapScale/(1.0+spacing));
}
Vector2 gridToMinimap(Vector2 grid) {
	return Vector2(grid.x/mapSize.x*minimapSize.x, 540-grid.y/mapSize.y*minimapSize.y);
}

class FpsTracker;
class PlayerController;
class BoxCollider;

Engine* engine;
FpsTracker* tracker;
PlayerController* playerController;
StencilSimple lightStencil;

OrthoCam* cam;
OrthoCam* uiCam;

Shader* playerShader;
Shader* flashlightShader;
Shader* playerIconShader;
Shader* backgroundShader;
Shader* instanceUnlitShader;
Shader* instanceWorkingShader;
Shader* instanceBrokenShader;
Shader* minimapShader;
Shader* lineShader;
Shader* textShader;

SpriteRenderer* playerRenderer;
SpriteRenderer* flashlightRenderer;
SpriteRenderer* playerIconRenderer;

std::vector<Renderer*> sceneRenderers;
std::vector<SpriteRenderer*> instanceStateRenderers;
std::vector<Renderer*> uiRenderers;
std::vector<TextRenderer*> debugText;

BoxCollider* playerCollider;
std::vector<BoxCollider*> instanceColliders;
class FpsTracker : Object {
	protected:
	double lastFrames[60]={};
	public:
	int avgFps=0;
	int highFps=0;
	int lowFps=0;
	float frameTime=0;
	FpsTracker() : Object() {}
	FpsTracker(Engine* _engine);
	void on_loop(double delta) override;
	int getAvgFps();
	int getHighFps();
	int getLowFps();
	float getFrameTime();
};
class PlayerController : public Object {
	protected:
	SpriteRenderer* playerRenderer;
	BoxCollider* playerCollider;
	SpriteRenderer* flashlightRenderer;
	SpriteRenderer* playerIconRenderer;
	OrthoCam* sceneCam;
	public:
	PlayerController() : Object(), playerRenderer(nullptr), playerCollider(nullptr), flashlightRenderer(nullptr), playerIconRenderer(nullptr), sceneCam(nullptr) {}
	PlayerController(Engine* _engine, SpriteRenderer* _playerRenderer, BoxCollider* _playerCollider, SpriteRenderer* _flashlightRenderer, SpriteRenderer* _playerIconRenderer, OrthoCam* _sceneCam);
	int inputs[5]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void on_loop(double delta) override;
	void setPos(Vector2 pos);
	Vector2 getPos();
};
bool ColliderDebug = false;
struct CollitionData {
	Vector2 normal;
	float dist;
	CollitionData(Vector2 _normal,float _dist) : normal(_normal), dist(_dist) {}
};
class BoxCollider : public Object{
	public:
	Vector2 pos;
	Vector2 size;
	float boundingRadius;
	Shader* debugLineShader;
	LineRenderer* debugRenderer;
	BoxCollider() : Object(), pos(Vector2(0)), size(Vector2(0)), boundingRadius(0.0f), debugLineShader(nullptr), debugRenderer(nullptr) {}
	BoxCollider(Engine* _engine, Vector2 _pos, Vector2 _size, Shader* _debugLineShader);
	void drawOutline();
	void on_delete() override;
	CollitionData checkCollision(BoxCollider* other);
};

int main(int argc, char** argv);
void Loop(double delta);
void onLateDelete();
#endif// MAINH