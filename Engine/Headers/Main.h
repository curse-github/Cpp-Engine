#pragma once
#ifndef _MAIN_H
#define _MAIN_H

#include "Json.h"
#include "GameLib.h"
#include "Engine.h"
#include "BatchedRenderers.h"

#include <math.h>
#include <array>
#define PI 3.14159265
#define TAU 6.2831853

Engine* engine;
FpsTracker* tracker;
std::unique_ptr<Pathfinder> finder;
class Player;
class Enemy;
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
Shader* textShader;

std::vector<Renderer2D*> sceneRenderers;
std::vector<Renderer*> uiRenderers;
std::vector<TextRenderer*> debugText;
StaticBatchedSpriteRenderer* instanceRenderer;
StaticBatchedSpriteRenderer* instanceStateRenderer;

class Player : public Object, public Transform2D {
	protected:
	SpriteRenderer* renderer;
	StencilSimple flashlightStencil;
	OrthoCam* sceneCam;
	SpriteRenderer* flashlightRenderer;
	SpriteRenderer* iconRenderer;
	int inputs[5]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	public:
	BoxCollider* collider;
	Player() : Object(), Transform2D(), renderer(nullptr), collider(nullptr), flashlightStencil(StencilSimple()), sceneCam(nullptr), flashlightRenderer(nullptr), iconRenderer(nullptr) {}
	Player(Engine* _engine, OrthoCam* _sceneCam, const Vector2& _position, Shader* playerShader, Shader* flashlightShader, Shader* iconShader);
	void flashlightStencilOn();
	void flashlightStencilOff();
	void setPos(const Vector2& pos);
};
class Enemy : public Object, public Transform2D {
	protected:
	SpriteRenderer* renderer;
	BoxCollider* collider;
	SpriteRenderer* iconRenderer;
	Shader* lineShader;
	Pathfinder* pathfinder;
	Player* target;
	Vector2 lastSpottedPos;
	std::vector<Vector2> path;
	void setDebugLine(std::vector<Vector2> line);
	void on_loop(const double& delta) override;
	public:
	LineRenderer* debugRen=nullptr;
	Enemy() : Object(), Transform2D(), renderer(nullptr), collider(nullptr), iconRenderer(nullptr), lineShader(nullptr), pathfinder(nullptr), target(nullptr) {}
	Enemy(Engine* _engine, const Vector2& _position, Shader* enemyShader, Shader* iconShader, Shader* _lineShader, Pathfinder* _pathfinder, Player* _target);
};

Vector2 HD1080P(1920.0, 1080.0);
Vector2 viewRange(480.0f, 270.0f);
int main(int argc, char** argv);
void Loop(const double& delta);
#endif// _MAIN_H