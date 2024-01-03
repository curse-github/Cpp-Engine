#pragma once
#ifndef _UI_H
#define _UI_H

#include "Engine.h"

class Clickable;
class UiHandler : Object {
	protected:
	std::vector<Clickable*> clickables;
	friend Clickable;
	void on_mouse(const double& mouseX, const double& mouseY) override;
	void on_mouse_button(const int& button, const int& action, const int& mods) override;
	public:
	static UiHandler* instance;
	UiHandler();
	UiHandler(UiHandler& copy)=delete;
	UiHandler(UiHandler&& move)=delete;
	void operator=(const UiHandler& other)=delete;

	void draw();
};
class Clickable : virtual public hasTransform2D {
	protected:
	Clickable() : cam(nullptr), hasTransform2D() {};
	Clickable(OrthoCam* _cam);
	virtual ~Clickable();
	virtual void on_click(const Vector2& pos);
	virtual void on_release(const Vector2& pos);
	virtual void on_hover(const Vector2& pos);
	virtual void on_unhover(const Vector2& pos);
	friend UiHandler;
	public:
	OrthoCam* cam;
	bool isPressed=false;
	bool isHover=false;
};
typedef std::function<void()> buttonfun;
class Button : virtual public hasTransform2D, public Clickable {
	protected:
	void on_click(const Vector2& pos) override;
	void on_release(const Vector2& pos) override;
	void on_hover(const Vector2& pos) override;
	void on_unhover(const Vector2& pos) override;
	public:
	Vector4 color;
	Vector4 hoverColor;
	Vector4 pressedColor;
	Shader* shader;
	SpriteRenderer* renderer;
	buttonfun onclick;
	Button() : hasTransform2D(), Clickable() {};
	Button(OrthoCam* _cam, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
#endif