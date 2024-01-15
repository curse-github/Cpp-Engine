#pragma once
#ifndef _MAIN_H
#define _MAIN_H

#include "Json.h"
#include "GameLib.h"
#include "Engine.h"
#include "BatchedRenderers.h"
#include "UI.h"

class Player;
class Enemy;
class Instance;

Engine* engine;
UiHandler* uiHandler;

Player* player;
std::unique_ptr<Pathfinder> finder;
Enemy* enemy;

OrthoCam* cam;
OrthoCam* uiCam;

Shader* lineShader;

BatchedSpriteRenderer* spriteRenderer;
StaticBatchedSpriteRenderer* staticSpriteRenderer;
StaticBatchedSpriteRenderer* instanceStateSpritesRenderer;

SpritesheetAnimationRenderer* test;

BatchedTextData* fpsText;
#ifdef _DEBUG
BatchedTextData* debugText;
#endif// _DEBUG

class Player : public Object, public hasTransform2D {
	protected:
	OrthoCam* sceneCam;
	BatchedQuadData* renderer;
	DotRenderer* flashlightRenderer;
	StencilSimple flashlightStencil;
	BatchedQuadData* iconRenderer;
	int inputs[5]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	public:
	BoxCollider* collider;
	Player(OrthoCam* _sceneCam, const Vector3& playerModulate, Texture* playerTex, const Vector4& flashlightColor, const Vector2& _position=Vector2::ZERO);
	void flashlightStencilOn();
	void flashlightStencilOff();
	void setPos(const Vector2& pos);
};
class Enemy : public Object, public hasTransform2D {
	protected:
	BatchedQuadData* renderer;
	BoxCollider* collider;
	BatchedQuadData* iconRenderer;
	Pathfinder* pathfinder;
	hasTransform2D* target;
	Vector2 lastSpottedPos;
	std::vector<Vector2> path;
	void on_loop(const double& delta) override;
	public:
	LineRenderer* debugRen=nullptr;
	Enemy(const Vector3& enemyModulate, Texture* enemyTex, Pathfinder* _pathfinder, hasTransform2D* _target, const Vector2& _position=Vector2::ZERO);
};
ClickDetector* instanceClickDetector;
class Instance : virtual public hasTransform2D, public Clickable {
	protected:
	bool broken=false;
	BatchedQuadData* stateQuad;
	Texture* instanceWorkingTex;
	Texture* instanceBrokenTex;
	virtual void on_click(const Vector2& pos) override;
	virtual void on_release(const Vector2& pos) override {};
	virtual void on_hover(const Vector2& pos) override {};
	virtual void on_unhover(const Vector2& pos) override {};
	public:
	Instance(ClickDetector* clickDetector, Texture* _instanceUnlitTex, Texture* _instanceWorkingTex, Texture* _instanceBrokenTex, const Vector2& _position=Vector2::ZERO, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
	void fixInstance();
	void breakInstance();
};

Vector2 HD1080P(1920.0, 1080.0);
Vector2 viewRange(480.0f, 270.0f);
int Run();
void Render(const double& delta);
int main(int argc, char** argv);
#endif// _MAIN_H