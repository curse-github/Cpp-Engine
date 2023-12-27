#include "UI.h"

std::vector<Clickable*> clickables;
UiHandler* UiHandler::instance=nullptr;
UiHandler::UiHandler() : Object() {
	instance=this;
}
void UiHandler::on_mouse(const double& mouseX, const double& mouseY) {
	Vector2 pos(mouseX, mouseY);
	Log(pos);
	for(Clickable* c:clickables) {
		//detect if you clicked on the clickable
	}
}
void UiHandler::draw() {

}

Clickable::Clickable(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor) : Transform2D(_position, _zIndex, _scale, _anchor, 0.0f) {
	clickables.push_back(this);
}
void Clickable::onClick(Vector2 pos) {

}