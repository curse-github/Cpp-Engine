#pragma once
#ifndef _UI_H
#define _UI_H

#include "Engine.h"
#include "BatchedRenderers.h"

class Clickable;
class ClickDetector : public Object {
	protected:
	std::vector<Clickable*> clickables;
	friend Clickable;
	void on_mouse(const double& mouseX, const double& mouseY) override;
	void on_mouse_button(const int& button, const int& action, const int& mods) override;
	public:
	OrthoCam* cam;
	ClickDetector(OrthoCam* _cam);
};
class Clickable : virtual public hasTransform2D {
	protected:
	Clickable() : hasTransform2D(), detector(nullptr) { initialized=false; };
	Clickable(ClickDetector* _detector);
	virtual ~Clickable();
	virtual void on_click(const Vector2& pos)=0;
	virtual void on_release(const Vector2& pos)=0;
	virtual void on_hover(const Vector2& pos)=0;
	virtual void on_unhover(const Vector2& pos)=0;
	friend ClickDetector;
	public:
	bool initialized=false;
	ClickDetector* detector;
	bool isPressed=false;
	bool isHover=false;
};

class Button;
class UiHandler : public Object {
	protected:
	void on_start();
	public:
	static UiHandler* instance;
	OrthoCam* cam;
	UiHandler() : cam(nullptr), spriteRenderer(nullptr), textRenderer(nullptr), clickableHandler(nullptr) { initialized=false; };
	UiHandler(OrthoCam* _cam);
	UiHandler(UiHandler& copy)=delete;
	UiHandler(UiHandler&& move)=delete;
	void operator=(const UiHandler& other)=delete;

	BatchedSpriteRenderer* spriteRenderer;
	BatchedTextRenderer* textRenderer;
	ClickDetector* clickableHandler;

	std::vector<Button*> buttons;

	BatchedQuadData* Sprite(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* Quad(const Vector4& _modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedTextData* Text(const std::string& _text, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	Button* createButton(const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw();
};
typedef std::function<void()> buttonfun;
class Button : virtual public hasTransform2D, public Clickable {
	protected:
	void on_click(const Vector2& pos) override;
	void on_release(const Vector2& pos) override;
	void on_hover(const Vector2& pos) override;
	void on_unhover(const Vector2& pos) override;
	public:
	UiHandler* handler;
	BatchedQuadData* quad;
	Vector4 color;
	Vector4 hoverColor;
	Vector4 pressedColor;

	buttonfun onclick;
	buttonfun onrelease;
	buttonfun onhover;
	buttonfun onunhover;

	Button() : hasTransform2D(), Clickable(), handler(nullptr), quad(nullptr) { initialized=false; };
	Button(UiHandler* _handler, ClickDetector* _detector, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
#endif