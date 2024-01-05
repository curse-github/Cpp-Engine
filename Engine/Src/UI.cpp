#include "UI.h"

#pragma region ClickDetector
void ClickDetector::on_mouse(const double& mouseX, const double& mouseY) {
	if(Engine::instance->ended||!initialized) return;
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 camPosition=cam->getWorldPos();
	Vector2 camScale=cam->scale;
	Vector2 relPos=camPosition+Vector2((pos.x/res.x-0.5f)*camScale.x, (0.5f-pos.y/res.y)*camScale.y);
	for(Clickable* c:clickables) {
		if(!c->inRange(cam->getWorldPos(), cam->scale)) continue;
		//detect if you hovered over the clickable
		bool wasHovered=c->isHover;
		c->isHover=c->inRange(relPos, 0);
		if(c->isHover) {
			if(!wasHovered) c->on_hover(relPos);
		} else {
			if(wasHovered) { c->on_unhover(relPos); c->on_release(relPos); }
		}
	}
}
void ClickDetector::on_mouse_button(const int& button, const int& action, const int& mods) {
	if(Engine::instance->ended||!initialized) return;
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 camPosition=cam->getWorldPos();
	Vector2 camScale=cam->scale;
	Vector2 relPos=camPosition+Vector2((pos.x/res.x-0.5f)*camScale.x, (0.5f-pos.y/res.y)*camScale.y);
	for(Clickable* c:clickables) {
		if(!c->inRange(cam->getWorldPos(), cam->scale)) continue;
		//detect if you clicked on the clickable
		if(action==GLFW_PRESS) {
			if(c->inRange(relPos, 0.0f)) { c->isPressed=true; c->on_click(relPos); }
		} else if(action==GLFW_RELEASE) {
			if(c->isPressed&&c->inRange(relPos, 0.0f)) { c->isPressed=false; c->on_release(relPos); }
		}
	}
}
ClickDetector::ClickDetector(OrthoCam* _cam) : Object(), cam(_cam) {
	if(Engine::instance->ended||!initialized) return;
	Engine::instance->sub_mouse(this);
	Engine::instance->sub_mouse_button(this);
}
#pragma endregion// ClickDetector
#pragma region Clickable
Clickable::Clickable(ClickDetector* _detector) : hasTransform2D(), detector(_detector) {
	if(Engine::instance->ended||_detector==nullptr||!_detector->initialized) { initialized=false;return; }
	initialized=true;
	detector->clickables.push_back(this);
}
Clickable::~Clickable() {
	if(!initialized||detector==nullptr) return;
	unsigned int size=static_cast<unsigned int>(detector->clickables.size());
	for(unsigned int i=0; i<size; i++) if(detector->clickables[i]==this) detector->clickables.erase(detector->clickables.begin()+i);
}
#pragma endregion// Clickable

#pragma region UiHandler
void UiHandler::on_start() {}
UiHandler* UiHandler::instance=nullptr;
UiHandler::UiHandler(OrthoCam* _cam) :
	Object(), cam(_cam), clickableHandler(new ClickDetector(_cam)), spriteRenderer(new BatchedSpriteRenderer(_cam)), textRenderer(new BatchedTextRenderer(_cam)) {
	if(Engine::instance->ended||!initialized||cam==nullptr||!cam->initialized||
		!clickableHandler->initialized||!spriteRenderer->initialized) return;
	instance=this;
	Engine::instance->sub_start(this);
}
BatchedQuadData* UiHandler::Sprite(const Vector4& _modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	return spriteRenderer->addSprite(_modulate, tex, _position, _zIndex, _scale, _anchor);
}
BatchedQuadData* UiHandler::Quad(const Vector4& _modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	return spriteRenderer->addQuad(_modulate, _position, _zIndex, _scale, _anchor);
}
BatchedTextData* UiHandler::Text(const std::string& _text, const Vector4& color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	return textRenderer->addText(_text, color, _position, _zIndex, _scale, _anchor);
}
Button* UiHandler::createButton(const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	if(Engine::instance->ended||!initialized) return nullptr;
	Button* tmp=new Button(this, this->clickableHandler, _color, _hoverColor, _pressedColor, _position, _zIndex, _scale, _anchor);
	buttons.push_back(tmp);
	return tmp;
}
void UiHandler::draw() {
	if(Engine::instance->ended||!initialized) return;
	spriteRenderer->draw();
	textRenderer->draw();
}
#pragma endregion// UiHandler
#pragma region Button
void Button::on_click(const Vector2& pos) {
	if(Engine::instance->ended||!initialized) return;
	quad->modulate=pressedColor;
	if(onclick) onclick();
}
void Button::on_release(const Vector2& pos) {
	if(Engine::instance->ended||!initialized) return;
	if(isHover) quad->modulate=hoverColor; else quad->modulate=color;
	if(onrelease) onrelease();
}
void Button::on_hover(const Vector2& pos) {
	if(Engine::instance->ended||!initialized) return;
	quad->modulate=hoverColor;
	if(onhover) onhover();
}
void Button::on_unhover(const Vector2& pos) {
	if(Engine::instance->ended||!initialized) return;
	quad->modulate=color;
	if(onunhover) onunhover();
}
Button::Button(UiHandler* _handler, ClickDetector* _detector, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) :
	hasTransform2D(_position, _zIndex, _scale, _anchor), Clickable(_detector), handler(_handler), quad(nullptr), color(_color), hoverColor(_hoverColor), pressedColor(_pressedColor) {
	if(Engine::instance->ended||!initialized) { return;initialized=false; }
	quad=handler->Quad(color, Vector2::ZERO, _zIndex, Vector2::ONE, _anchor);
	addChild(quad);
}
#pragma endregion// Button