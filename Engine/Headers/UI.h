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
	Clickable(ClickDetector* _detector);
	virtual ~Clickable();
	virtual void on_click(const Vector2& pos)=0;
	virtual void on_release(const Vector2& pos)=0;
	virtual void on_hover(const Vector2& pos)=0;
	virtual void on_unhover(const Vector2& pos)=0;
	friend ClickDetector;
	public:
	ClickDetector* detector;
	bool isPressed=false;
	bool isHover=false;
};

class UiElement;
class Button;
class TextInput;
class Checkbox;
class HorizontalSlider;

class UiHandler : public Object {
	protected:
	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_loop(const double& delta) override;
	void on_mouse(const double &mouseX, const double &mouseY) override;
	void selectElement(UiElement* el);
	friend UiElement;
	public:
	static UiHandler* instance;
	OrthoCam* cam;
	UiElement* selected=nullptr;//what object is currently selected
	UiHandler(OrthoCam* _cam);
	UiHandler(UiHandler& copy)=delete;// UiHandler handler(other);, or UiHandler handler = other;
	UiHandler(UiHandler&& move)=delete;// UiHandler handler((UiHandler&&)other);, or UiHandler handler = (UiHandler&&)other;
	void operator=(const UiHandler& copy)=delete;// handler = other; where handler and other are 'UiHandler'
	void operator=(UiHandler&& move)=delete;// handler = (UiHandler&&)other; where handler and other are 'UiHandler'

	BatchedSpriteRenderer* spriteRenderer;
	BatchedTextRenderer* textRenderer;
	BatchedDotRenderer *dotRenderer;
	ClickDetector* clickableHandler;

	std::vector<UiElement*> uiElements;

	BatchedQuadData* Sprite(const Vector4& _modulate, Texture* tex, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedQuadData* Quad(const Vector4& _modulate, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	BatchedTextData* Text(const std::string& _text, const Vector4& color, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const float& _scale=1.0f, const Vector2& _anchor=Vector2::Center);
	BatchedDotData *TexturedDot(const Vector4 &modulate, Texture *tex, const float &radius, const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
	BatchedDotData *Dot(const Vector4 &modulate, const float &radius, const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
	Button* createButton(const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
	TextInput *createTextInput(const std::string &_value="", const std::string &_placeholder="", const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
	Checkbox *createCheckbox(const Vector4 &_checkedColor, const Vector4 &_uncheckedColor, const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
	void draw();
};

class UiElement : virtual public hasTransform2D, public Clickable {
	protected:
	virtual void on_key(const int& key, const int& scancode, const int& action, const int& mods) {};
	virtual void on_loop(const double& delta) {};
	virtual void on_mouse(const double &mouseX, const double &mouseY) {};
	friend UiHandler;

	UiElement(UiHandler* _handler, ClickDetector* _detector) :
		hasTransform2D(), Clickable(_detector), handler(_handler) {
		engine_assert(handler!=nullptr, "[UiElement]: handler is nullptr");
		_handler->uiElements.push_back(this);
	}

	virtual void on_click(const Vector2& pos)=0;
	void on_release(const Vector2& pos) override {
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

	Button(UiHandler* _handler, ClickDetector* _detector, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
class TextInput : virtual public hasTransform2D, public UiElement {
	protected:
	void on_click(const Vector2& pos) override;
	void on_release(const Vector2& pos) override;
	void on_hover(const Vector2& pos) override;
	void on_unhover(const Vector2& pos) override;
	void on_loop(const double& delta) override;

	void on_key(const int& key, const int& scancode, const int& action, const int& mods) override;
	public:
	BatchedQuadData* quad;
	BatchedTextData* text;
	std::string placeholder="";
	std::string value="";
	bool clearOnEnter=false;
	typedef std::function<void(std::string)> enterfun;
	void Submit();
	enterfun onenter;
	void update();

	TextInput(UiHandler* _handler, ClickDetector* _detector, const std::string& _value="", const std::string& _placeholder="", const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center);
};
class Checkbox : virtual public hasTransform2D, public UiElement {
	protected:
	void on_click(const Vector2 &pos) override;
	void on_release(const Vector2 &pos) override;
	void on_hover(const Vector2 &pos) override;
	void on_unhover(const Vector2 &pos) override;

	void on_key(const int &key, const int &scancode, const int &action, const int &mods) override;
	BatchedQuadData *quad;
	public:
	typedef std::function<void()> voidfun;
	Vector4 uncheckedColor;
	Vector4 checkedColor;

	voidfun oncheck;
	voidfun onuncheck;
	voidfun onhover;
	voidfun onunhover;

	bool checked=false;
	Checkbox(UiHandler *_handler, ClickDetector *_detector, const Vector4 &_checkedColor, const Vector4 &_uncheckedColor, const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
};
class HorizontalSlider : virtual public hasTransform2D, public UiElement {
	protected:
	void on_click(const Vector2 &pos) override {};
	//on_release override is not needed
	void on_hover(const Vector2 &pos) override {};//not needed, but overload is required
	void on_unhover(const Vector2 &pos) override {};//not needed, but overload is required

	void on_mouse(const double &mouseX, const double &mouseY) override;
	BatchedQuadData *quad;
	BatchedDotData *dot;
	float value;
	public:
	typedef std::function<void(float)> changefun;
	Vector4 barColor;
	Vector4 sliderColor;
	float maxValue=1;
	float minValue=0;

	changefun onchange;

	HorizontalSlider(UiHandler *_handler, ClickDetector *_detector, const Vector4 &_barColor, const Vector4 &_sliderColor, const float &_value, const Vector2 &_position=Vector2::ZERO, const float &_zIndex=0.0f, const Vector2 &_scale=Vector2::ONE, const Vector2 &_anchor=Vector2::Center);
	void setValue(float _value);
	float getValue();
};
#endif