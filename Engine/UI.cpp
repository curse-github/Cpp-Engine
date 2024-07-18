#include "UI.h"

#pragma region ClickDetector
void ClickDetector::on_mouse(const double& mouseX, const double& mouseY) {
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 camScale=cam->scale;
	Vector2 relPos=cam->getWorldPos()+Vector2((pos.x/res.x-0.5f)*camScale.x, (0.5f-pos.y/res.y)*camScale.y);
	for(Clickable* c:clickables) {
		if(!c->inRange(cam->getWorldPos(), cam->scale)) continue;
		//detect if you hovered over a clickable
		bool wasHovered=c->isHover;
		c->isHover=c->inRange(relPos, 0);
		if(c->isHover) {
			if(!wasHovered) c->on_hover(relPos);
		} else {
			if(wasHovered) { c->on_unhover(relPos); if(c->isPressed) { c->isPressed=false; c->on_release(relPos); } }
		}
	}
}
void ClickDetector::on_mouse_button(const int& button, const int& action, const int& mods) {
	Vector2 pos=Engine::instance->curMousePos;
	Vector2 res=Engine::instance->curResolution;
	Vector2 camScale=cam->scale;
	Vector2 relPos=cam->getWorldPos()+Vector2((pos.x/res.x-0.5f)*camScale.x, (0.5f-pos.y/res.y)*camScale.y);
	if(action==GLFW_PRESS) {
		bool pressedAny=false;
		for(Clickable* c:clickables) {
			// detect if you clicked on a clickable
			if(c->inRange(relPos, 0.0f)) { pressedAny=true; c->isPressed=true; c->on_click(relPos); }
		}
		if(!pressedAny) if(on_click_background) on_click_background();
	} else if(action==GLFW_RELEASE) {
		for(Clickable* c:clickables) {
			// detect if you clicked on a clickable
			if(c->isPressed&&c->inRange(relPos, 0.0f)) { c->isPressed=false; c->on_release(relPos); }
		}
	}
}
ClickDetector::ClickDetector(OrthoCam* _cam) : Object(), cam(_cam) {
	engine_assert(cam!=nullptr, "[ClickDetector]: cam is nullptr");
	Engine::instance->sub_mouse(this);
	Engine::instance->sub_mouse_button(this);
}
#pragma endregion// ClickDetector
#pragma region Clickable
Clickable::Clickable(ClickDetector* _detector) : hasTransform2D(), detector(_detector) {
	engine_assert(detector!=nullptr, "[Clickable]: detector is nullptr");
	detector->clickables.push_back(this);
}
Clickable::~Clickable() {
	unsigned int size=static_cast<unsigned int>(detector->clickables.size());
	for(unsigned int i=0; i<size; i++) if(detector->clickables[i]==this) detector->clickables.erase(detector->clickables.begin()+i);
}
#pragma endregion// Clickable

#pragma region UiHandler
void UiHandler::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(selected!=nullptr) {
		if(key==GLFW_KEY_TAB && selected->selectOnTab!=nullptr) selectElement(selected->selectOnTab);
		else selected->on_key(key, scancode, action, mods);
	}
}
void UiHandler::on_loop(const double& delta) {
	for(UiElement* el:uiElements) {
		el->on_loop(delta);
	}
}
void UiHandler::on_mouse(const double &mouseX, const double &mouseY) {
	for(UiElement *el:uiElements) {
		if (el->isPressed) el->on_mouse(mouseX,mouseY);
	}
}
UiHandler* UiHandler::instance=nullptr;
UiHandler::UiHandler(OrthoCam* _cam) :
	Object(), cam(_cam), clickableHandler(new ClickDetector(_cam)), spriteRenderer(new BatchedSpriteRenderer(_cam)), textRenderer(new BatchedTextRenderer(_cam)), dotRenderer(new BatchedDotRenderer(_cam)) {
	engine_assert(cam!=nullptr, "[UiHandler]: cam is nullptr");
	instance=this;
	Engine::instance->sub_key(this);
	Engine::instance->sub_loop(this);
	Engine::instance->sub_mouse(this);
	clickableHandler->on_click_background=[&]() {
		unselect();
	};
}
BatchedQuadData* UiHandler::Sprite(const Vector4& _modulate, Texture* tex, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	return spriteRenderer->addSprite(_modulate, tex, _position, _zIndex, _scale, _anchor);
}
BatchedQuadData* UiHandler::Quad(const Vector4& _modulate, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	return spriteRenderer->addQuad(_modulate, _position, _zIndex, _scale, _anchor);
}
BatchedTextData* UiHandler::Text(const std::string& _text, const Vector4& color, const Vector2& _position, const float& _zIndex, const float& _scale, const Vector2& _anchor) {
	return textRenderer->addText(_text, color, _position, _zIndex, _scale, _anchor);
}
BatchedDotData *UiHandler::TexturedDot(const Vector4 &modulate, Texture *tex, const float &radius, const Vector2 &_position, const float &_zIndex, const Vector2 &_scale, const Vector2 &_anchor) {
	return dotRenderer->addTexturedDot(modulate, tex, radius, _position, _zIndex, _scale, _anchor);
}
BatchedDotData *UiHandler::Dot(const Vector4 &modulate, const float &radius, const Vector2 &_position, const float &_zIndex, const Vector2 &_scale, const Vector2 &_anchor) {
	return dotRenderer->addDot(modulate, radius, _position, _zIndex, _scale, _anchor);
}
Button* UiHandler::createButton(const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	return new Button(this, this->clickableHandler, _color, _hoverColor, _pressedColor, _position, _zIndex, _scale, _anchor);
}
TextInput* UiHandler::createTextInput(const std::string& _value, const std::string& _placeholder, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) {
	return new TextInput(this, this->clickableHandler, _value, _placeholder, _position, _zIndex, _scale, _anchor);
}
Checkbox *UiHandler::createCheckbox(const Vector4 &_checkedColor, const Vector4 &_uncheckedColor, const Vector2 &_position, const float &_zIndex, const Vector2 &_scale, const Vector2 &_anchor) {
	return new Checkbox(this, this->clickableHandler, _checkedColor, _uncheckedColor, _position, _zIndex, _scale, _anchor);
}
void UiHandler::draw() {
	glClear(GL_DEPTH_BUFFER_BIT);
	spriteRenderer->draw();
	textRenderer->draw();
	dotRenderer->draw();
}
void UiHandler::selectElement(UiElement* el) {
	if(selected!=nullptr) selected->selected=false;
	selected=el;
	el->selected=true;
}
void UiHandler::unselect() {
	if(this->selected!=nullptr) {
		this->selected->selected=false;
		this->selected=nullptr;
	}
}
#pragma endregion// UiHandler
#pragma region Button
void Button::on_click(const Vector2& pos) {
	quad->modulate=pressedColor;
	if(onclick) onclick();
}
void Button::on_release(const Vector2& pos) {
	UiElement::on_release(pos);
	if(isHover) quad->modulate=hoverColor; else quad->modulate=color;
	if(onrelease) onrelease();
}
void Button::on_hover(const Vector2& pos) {
	quad->modulate=hoverColor;
	if(onhover) onhover();
}
void Button::on_unhover(const Vector2& pos) {
	quad->modulate=color;
	if(onunhover) onunhover();
}
void Button::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(action>GLFW_RELEASE&&key==GLFW_KEY_ENTER) {
		on_click(Vector2::ZERO);
		on_release(Vector2::ZERO);
	}
}
Button::Button(UiHandler* _handler, ClickDetector* _detector, const Vector4& _color, const Vector4& _hoverColor, const Vector4& _pressedColor, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) :
	hasTransform2D(_position, _zIndex, _scale, _anchor), UiElement(_handler, _detector), quad(nullptr), color(_color), hoverColor(_hoverColor), pressedColor(_pressedColor) {
	quad=handler->Quad(color, Vector2::ZERO, _zIndex, Vector2::ONE, _anchor);
	addChild(quad);
}
#pragma endregion// Button
#pragma region TextInput
void TextInput::on_click(const Vector2& pos) {}
void TextInput::on_release(const Vector2& pos) {
	UiElement::on_release(pos);
}
void TextInput::on_hover(const Vector2& pos) {}
void TextInput::on_unhover(const Vector2& pos) {}
void TextInput::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(action>GLFW_RELEASE) {
		if(key>=GLFW_KEY_SPACE&&key<=GLFW_KEY_GRAVE_ACCENT) {
			if(key>=GLFW_KEY_A&&key<=GLFW_KEY_Z) {// letter values
				bool shift=(mods&GLFW_MOD_SHIFT)>0;
				bool capsLock=(mods&GLFW_MOD_CAPS_LOCK)>0;
				value+=static_cast<char>(key+((shift^capsLock) ? 0 : 32));// shift character by 32 if not caps
			} else {// non letter values
				if((mods&GLFW_MOD_SHIFT)>0) {
					switch(key) {
						case GLFW_KEY_SPACE: value+=' '; break;// ' '
						case GLFW_KEY_APOSTROPHE: value+='"'; break;// '\''
						case GLFW_KEY_COMMA: value+='<'; break;// ','
						case GLFW_KEY_MINUS: value+='_'; break;// '-'
						case GLFW_KEY_PERIOD: value+='>'; break;// '.'
						case GLFW_KEY_SLASH: value+='?'; break;// '/'
						case GLFW_KEY_1: value+='!'; break;// '1'
						case GLFW_KEY_2: value+='@'; break;// '2'
						case GLFW_KEY_3: value+='#'; break;// '3'
						case GLFW_KEY_4: value+='$'; break;// '4'
						case GLFW_KEY_5: value+='%'; break;// '5'
						case GLFW_KEY_6: value+='^'; break;// '6'
						case GLFW_KEY_7: value+='&'; break;// '7'
						case GLFW_KEY_8: value+='*'; break;// '8'
						case GLFW_KEY_9: value+='('; break;// '9'
						case GLFW_KEY_0: value+=')'; break;// '0'
						case GLFW_KEY_SEMICOLON: value+=':'; break;// ';'
						case GLFW_KEY_EQUAL: value+='+'; break;// '='
						case GLFW_KEY_LEFT_BRACKET: value+='{'; break;// '['
						case GLFW_KEY_BACKSLASH: value+='|'; break;// '\\'
						case GLFW_KEY_RIGHT_BRACKET: value+='}'; break;// ']'
						case GLFW_KEY_GRAVE_ACCENT: value+='`'; break;// '`'
					}
				} else value+=static_cast<char>(key);
			}
		} else if(key==GLFW_KEY_BACKSPACE) {
			if(!value.empty()) value.erase(value.size()-1);
		} else if(key==GLFW_KEY_ENTER) {
			Submit();
		}
	}
}
void TextInput::Submit() {
	if(!value.empty()) {
		if(onenter) onenter(value);
		if(clearOnEnter) value="";
	}
}
void TextInput::on_loop(const double& delta) {
	if(selected) {
		const bool cursorShouldBeOn=(static_cast<unsigned int>(floor(glfwGetTime()*2))%2)==1;
		text->color=Vector4::ONE;
		if(!value.empty()) text->text=value+(cursorShouldBeOn?"|":"");// show current value
		else text->text=cursorShouldBeOn?"|":"";
	} else {
		if(!value.empty()) { text->text=value; text->color=Vector4::ONE; }// show current value in white
		else { text->text=placeholder; text->color=Vector4(Vector3(0.5f), 1.0f); }// show placeholder value in gray
	}
}
TextInput::TextInput(UiHandler* _handler, ClickDetector* _detector, const std::string& _value, const std::string& _placeholder, const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) :
	hasTransform2D(_position, _zIndex, _scale, _anchor), UiElement(_handler, _detector), quad(nullptr), text(nullptr), value(_value), placeholder(_placeholder) {
	quad=handler->Quad(Vector4(Vector3(0.25f), 1.0f), Vector2::ZERO, _zIndex, Vector2::ONE, _anchor);
	addChild(quad);
	Vector2 textPos=Vector2(-_scale.x*0.45f, 0.0f);
	float textZIndex=getZIndex()+1.0f;
	if(!value.empty()) text=handler->Text(value, Vector4::ONE, textPos, textZIndex, 2.0f, Vector2::LeftCenter);
	else text=handler->Text(placeholder, Vector4(Vector3(0.5f), 1.0f), textPos, textZIndex, 2.0f, Vector2::LeftCenter);
	quad->addChild(text);
}
#pragma endregion/ TextInput
#pragma region Checkbox
void Checkbox::on_click(const Vector2 &pos) {}
void Checkbox::on_release(const Vector2 &pos) {
	UiElement::on_release(pos);
	checked=!checked;
	quad->modulate=checked ? checkedColor : uncheckedColor;
	if(checked) { if(oncheck) oncheck(); } else { if(onuncheck)onuncheck(); }
}
void Checkbox::on_hover(const Vector2 &pos) {
	if(onhover) onhover();
}
void Checkbox::on_unhover(const Vector2 &pos) {
	if(onunhover) onunhover();
}
void Checkbox::on_key(const int &key, const int &scancode, const int &action, const int &mods) {
	if(action>GLFW_RELEASE&&key==GLFW_KEY_ENTER) {
		on_click(Vector2::ZERO);
		on_release(Vector2::ZERO);
	}
}
Checkbox::Checkbox(UiHandler *_handler, ClickDetector *_detector, const Vector4 &_checkedColor, const Vector4 &_uncheckedColor, const Vector2 &_position, const float &_zIndex, const Vector2 &_scale, const Vector2 &_anchor) :
	hasTransform2D(_position, _zIndex, _scale, _anchor), UiElement(_handler, _detector), quad(nullptr), uncheckedColor(_uncheckedColor), checkedColor(_checkedColor) {
	quad=handler->Quad(checked ? checkedColor : uncheckedColor, Vector2::ZERO, _zIndex, Vector2::ONE, _anchor);
	addChild(quad);
}
#pragma endregion// Checkbox
#pragma region HorizontalSlider
#define _ENGINE_DEBUG 1
void HorizontalSlider::on_mouse(const double &mouseX, const double &mouseY) {
	Vector2 res=Engine::instance->curResolution;
	OrthoCam* cam = detector->cam;
	Vector2 camScale=cam->scale;
	Vector2 mouse=cam->getWorldPos()+Vector2((mouseX/res.x-0.5f)*camScale.x, (0.5f-mouseY/res.y)*camScale.y);
	Vector2 quadPos = quad->getWorldPos();
	Vector2 quadScale = quad->getWorldScale();
	Vector2 transformScale = transform.scale;
	value = std::max(0.0f,std::min(((mouse.x-quadPos.x)/quadScale.x)+1.0f,1.0f));
	dot->position = Vector2((value-1.0f)*quadScale.x-transformScale.y/3.0f, 0.5f*transformScale.y);
	value=minValue+value*(maxValue-minValue);
	if (onchange) onchange(value);
}
HorizontalSlider::HorizontalSlider(UiHandler *_handler, ClickDetector *_detector, const Vector4 &_barColor, const Vector4 &_sliderColor, const float &_value, const Vector2 &_position, const float &_zIndex, const Vector2 &_scale, const Vector2 &_anchor) :
	hasTransform2D(_position, _zIndex, _scale, _anchor), UiElement(_handler, _detector), quad(nullptr), barColor(_barColor), sliderColor(_sliderColor), value(_value) {
	Vector2 barPos(0.0f-transform.scale.y/3.0f, 0.5f*transform.scale.y);
	Vector2 barSize(1.0f-transform.scale.y/1.5f/transform.scale.x, 1/3.0f);
	quad=handler->Quad(barColor, barPos, _zIndex, barSize, Vector2::RightCenter);
	addChild(quad);
	dot=handler->Dot(sliderColor, 1.0f, Vector2(0.0f, 0.5f*transform.scale.y), _zIndex+1.0f, Vector2((1/3.0f)/transform.scale.x*transform.scale.y, 1/3.0f), Vector2::Center);
	addChild(dot);
}
void HorizontalSlider::setValue(float _value) {
	value=_value;
	Vector2 quadScale = quad->getWorldScale();
	Vector2 transformScale = transform.scale;
	dot->position = Vector2((value/(maxValue-minValue)-minValue-1.0f)*quadScale.x-transformScale.y/3.0f, 0.5f*transformScale.y);
	if (onchange) onchange(value);
}
float HorizontalSlider::getValue() {
	return value;
}
#pragma endregion// HorizontalSlider
