#pragma once
#ifndef _MAIN_H
#define _MAIN_H

#include "Json.h"
#include "GameLib.h"
#include "Engine.h"
#include "BatchedRenderers.h"
#include "UI.h"

#define PI 3.14159265
#define TAU 6.2831853

class Player;
class Enemy;
class Instance;

Engine* engine;
UiHandler* uiHandler;

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

Shader* instanceShader;
Shader* instanceStateShader;

Shader* lineShader;

std::vector<Renderer2D*> sceneRenderers;
std::vector<Instance*> instances;
std::vector<Renderer*> uiRenderers;
BatchedTextRenderer* textRenderer;
BatchedTextData* fpsText;
#ifdef _DEBUG
BatchedTextData* debugText;
#endif// _DEBUG
StaticBatchedSpriteRenderer* instanceRenderer;
StaticBatchedSpriteRenderer* instanceStateRenderer;

class Player : public Object, public hasTransform2D {
	protected:
	SpriteRenderer* renderer;
	StencilSimple flashlightStencil;
	OrthoCam* sceneCam;
	SpriteRenderer* flashlightRenderer;
	SpriteRenderer* iconRenderer;
	int inputs[5]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	public:
	BoxCollider* collider;
	Player() : Object(), hasTransform2D(), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(nullptr), flashlightRenderer(nullptr), iconRenderer(nullptr) {}
	Player(OrthoCam* _sceneCam, Shader* playerShader, Shader* flashlightShader, Shader* iconShader, const Vector2& _position=Vector2::ZERO);
	void flashlightStencilOn();
	void flashlightStencilOff();
	void setPos(const Vector2& pos);
};
class Enemy : public Object, public hasTransform2D {
	protected:
	SpriteRenderer* renderer;
	BoxCollider* collider;
	SpriteRenderer* iconRenderer;
	Shader* lineShader;
	Pathfinder* pathfinder;
	hasTransform2D* target;
	Vector2 lastSpottedPos;
	std::vector<Vector2> path;
	void setDebugLine(std::vector<Vector2> line);
	void on_loop(const double& delta) override;
	public:
	LineRenderer* debugRen=nullptr;
	Enemy() : Object(), hasTransform2D(), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(nullptr), pathfinder(nullptr), target(nullptr) {}
	Enemy(Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, hasTransform2D* _target, const Vector2& _position=Vector2::ZERO);
};
class Instance : virtual public hasTransform2D, public Clickable {
	protected:
	bool broken=false;
	BatchedQuadData* stateQuad;
	StaticBatchedSpriteRenderer* instanceStateRenderer;
	void on_click(const Vector2& pos) override;
	public:
	Instance() : hasTransform2D(), Clickable(), stateQuad(nullptr), instanceStateRenderer(nullptr) {};
	Instance(OrthoCam* _cam, Shader* lineShader, StaticBatchedSpriteRenderer* _instanceRenderer, StaticBatchedSpriteRenderer* _instanceStateRenderer, const Vector2& _position=Vector2::ZERO, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
	void fixInstance();
	void breakInstance();
};

Vector2 HD1080P(1920.0, 1080.0);
Vector2 viewRange(480.0f, 270.0f);
int Run();
void Loop(const double& delta);
void close();
int main(int argc, char** argv);
#endif// _MAIN_H