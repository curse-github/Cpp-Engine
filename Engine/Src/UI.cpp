#include "UI.h"

#pragma region UiHandler
void UiHandler::on_mouse(const double& mouseX, const double& mouseY) {
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 screenPos(pos.x/res.x-0.5f, 0.5f-pos.y/res.y);
	for(Clickable* c:clickables) {
		if(!c->inRange(c->cam->getWorldPos(), c->cam->scale)) continue;
		//detect if you hovered over the clickable
		Vector2 camPosition=c->cam->getWorldPos();
		Vector2 camScale=c->cam->scale;
		Vector2 relPos=camPosition+Vector2(screenPos.x*camScale.x, screenPos.y*camScale.y);
		bool wasHovered=c->isHover;
		c->isHover=c->inRange(relPos, 0);
		if(c->isHover) {
			if(!wasHovered) c->on_hover(relPos);
		} else {
			if(wasHovered) { c->on_unhover(relPos); c->on_release(relPos); }
		}
	}
}
void UiHandler::on_mouse_button(const int& button, const int& action, const int& mods) {
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 screenPos(pos.x/res.x-0.5f, 0.5f-pos.y/res.y);
	for(Clickable* c:clickables) {
		if(!c->inRange(c->cam->getWorldPos(), c->cam->scale)) continue;
		//detect if you clicked on the clickable
		Vector2 camPosition=c->cam->getWorldPos();
		Vector2 camScale=c->cam->scale;
		Vector2 relPos=camPosition+Vector2(screenPos.x*camScale.x, screenPos.y*camScale.y);
		if(action==GLFW_PRESS) {
			if(c->inRange(relPos, 0.0f)) { c->isPressed=true; c->on_click(relPos); }
		} else if(action==GLFW_RELEASE) {
			if(c->isPressed&&c->inRange(relPos, 0.0f)) { c->isPressed=false; c->on_release(relPos); }
		}
	}
}
UiHandler* UiHandler::instance=nullptr;
UiHandler::UiHandler() : Object() {
	instance=this;
	Engine::instance->sub_mouse(this);
	Engine::instance->sub_mouse_button(this);
}
void UiHandler::draw() {}
#pragma endregion// UiHandler

Clickable::Clickable(OrthoCam* _cam) : cam(_cam) {
	UiHandler::instance->clickables.push_back(this);
}
void Clickable::on_click(const Vector2& pos) {}
void Clickable::on_release(const Vector2& pos) {}
void Clickable::on_hover(const Vector2& pos) {}
void Clickable::on_unhover(const Vector2& pos) {}
Clickable::~Clickable() {
	unsigned int size=UiHandler::instance->clickables.size();
	for(unsigned int i=0; i<size; i++) if(UiHandler::instance->clickables[i]==this) UiHandler::instance->clickables.erase(UiHandler::instance->clickables.begin()+i);
}

void Button::on_click(const Vector2& pos) { shader->setFloat4("color", pressedColor); onclick(); }
void Button::on_release(const Vector2& pos) {
	if(isHover) shader->setFloat4("color", hoverColor); else shader->setFloat4("color", color);
}
void Button::on_hover(const Vector2& pos) { shader->setFloat4("color", hoverColor); }
void Button::on_unhover(const Vector2& pos) { shader->setFloat4("color", color); }
Button::Button(OrthoCam* _cam, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) :
	Clickable(_cam), Transform2D(_position, _zIndex, _scale, _anchor, 0.0f), color(_color), hoverColor(_hoverColor), pressedColor(_pressedColor), renderer(nullptr) {
	shader=new Shader("Shaders/basic.vert", "Shaders/color.frag");
	if(!shader->initialized)return;
	renderer=new SpriteRenderer(shader, Vector2::ZERO, _zIndex, Vector2::ONE, _anchor);
	if(!renderer->initialized)return;
	addChild(renderer);
	shader->setFloat4("color", color);
	cam->bindShader(shader);
	cam->use();
}