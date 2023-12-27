#pragma once
#ifndef _UI_H
#define _UI_H

#include "Engine.h"

class Clickable;
extern std::vector<Clickable*> clickables;
class UiHandler : Object {
	protected:
	static UiHandler* instance;
	public:
	UiHandler();
	UiHandler(UiHandler& copy)=delete;
	UiHandler(UiHandler&& move)=delete;
	void operator=(const UiHandler& other)=delete;
	void on_mouse(const double& mouseX, const double& mouseY) override;
	void draw();
};
class Clickable : public Transform2D {
	Clickable() : Transform2D() {};
	Clickable(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor);
	void onClick(Vector2 pos);
};

#endif