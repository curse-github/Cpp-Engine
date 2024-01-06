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
	typedef std::function<void()> voidfun;
	voidfun on_click_background;
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

class UiElement;
class Button;
class TextInput;
class UiHandler : public Object {
	protected:
	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	void selectElement(UiElement* el);
	friend UiElement;
	public:
	static UiHandler* instance;
	OrthoCam* cam;
	UiElement* selected=nullptr;//what object is currently selected
	UiHandler() : cam(nullptr), spriteRenderer(nullptr), textRenderer(nullptr), clickableHandler(nullptr) { initialized=false; };
	UiHandler(OrthoCam* _cam);
	UiHandler(UiHandler& copy)=delete;// UiHandler handler(other);, or UiHandler handler = other;
	UiHandler(UiHandler&& move)=delete;// UiHandler handler((UiHandler&&)other);, or UiHandler handler = (UiHandler&&)other;
	void operator=(const UiHandler& copy)=delete;// handler = other; where handler and other are 'UiHandler'
	void operator=(UiHandler&& move)=delete;// handler = (UiHandler&&)other; where handler and other are 'UiHandler'

	BatchedSpriteRenderer* spriteRenderer;
	BatchedTextRenderer* textRenderer;
	ClickDetector* clickableHandler;

	std::vector<UiElement*> uiElements;

	BatchedQuadData* Sprite(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* Quad(const Vector4& _modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedTextData* Text(const std::string& _text, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	Button* createButton(const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	TextInput* createTextInput(const std::string& _value="", const std::string& _placeholder="", const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	void draw();
};
class UiElement : virtual public hasTransform2D, public Clickable {
	protected:
	virtual void on_key(const int& key, const int& scancode, const int& action, const int& mods) {};
	virtual void on_loop(const double& delta) {};
	friend UiHandler;

	UiElement() : hasTransform2D(), Clickable(), handler(nullptr) { initialized=false; };
	UiElement(UiHandler* _handler, ClickDetector* _detector) :
		hasTransform2D(), Clickable(_detector), handler(_handler) {
		_handler->uiElements.push_back(this);
	}

	virtual void on_click(const Vector2& pos)=0;
	void on_release(const Vector2& pos) override {
		if(Engine::instance->ended||!initialized) return;
		handler->selectElement(this);
	}
	void on_hover(const Vector2& pos)=0;
	void on_unhover(const Vector2& pos)=0;
	public:
	UiHandler* handler;
	bool selected=false;
};
class Button : virtual public hasTransform2D, public UiElement {
	protected:
	void on_click(const Vector2& pos) override;
	void on_release(const Vector2& pos) override;
	void on_hover(const Vector2& pos) override;
	void on_unhover(const Vector2& pos) override;

	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	public:
	typedef std::function<void()> voidfun;
	BatchedQuadData* quad;
	Vector4 color;
	Vector4 hoverColor;
	Vector4 pressedColor;

	voidfun onclick;
	voidfun onrelease;
	voidfun onhover;
	voidfun onunhover;

	Button() : hasTransform2D(), UiElement(), quad(nullptr), color(Vector4::ZERO), hoverColor(Vector4::ZERO), pressedColor(Vector4::ZERO) { initialized=false; };
	Button(UiHandler* _handler, ClickDetector* _detector, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
class TextInput : virtual public hasTransform2D, public UiElement {
	protected:
	void on_click(const Vector2& pos) override;
	void on_release(const Vector2& pos) override;
	void on_hover(const Vector2& pos) override;
	void on_unhover(const Vector2& pos) override;

	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	public:
	BatchedQuadData* quad;
	BatchedTextData* text;
	std::string placeholder="";
	std::string value="";
	bool clearOnEnter=true;
	typedef std::function<void(std::string)> enterfun;
	enterfun onenter;
	void update();

	TextInput() : hasTransform2D(), UiElement(), quad(nullptr), text(nullptr) { initialized=false; };
	TextInput(UiHandler* _handler, ClickDetector* _detector, const std::string& _value="", const std::string& _placeholder="", const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
#endif