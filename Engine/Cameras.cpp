#include "Cameras.h"

#define clamp(a,b,c) std::max(b,std::min(a,c))

#pragma region Camera
Camera::Camera() : Object(), projection(Mat4x4()), view(Mat4x4()) {}
void Camera::update() {}
void Camera::bindShader(Shader* shader) {
	engine_assert(shader!=nullptr, "[Camera]: shader is nullptr");
	shaders.push_back(shader);
}
void Camera::bindShaders(const std::vector<Shader*>& shaders) {
	for(Shader* shdr:shaders) bindShader(shdr);
}
void Camera::use() {
	for(Shader* shdr:shaders) {
		if(shdr==nullptr) continue;
		shdr->setMat4x4("projection", projection);
		shdr->setMat4x4("view", view);
	}
}
#pragma endregion// Camera
#pragma region LookAtCam
LookAtCam::LookAtCam(const float& _aspect, const Vector3& _position, const Vector3& _focus) :
	Camera(), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), focus(_focus) {
	update();
}
void LookAtCam::update() {
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}
#pragma endregion// LookAtCam
#pragma region FreeCam
FreeCam::FreeCam(const float& _aspect, const Vector3& _position, const Vector3& _forward, const Vector3& _up) :
	Camera(), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), forward(_forward), up(_up) {
	Engine::instance->SetCursorMode(GLFW_CURSOR_DISABLED);
	Engine::instance->sub_key(this);
	Engine::instance->sub_scroll(this);
	Engine::instance->sub_mouse_delta(this);
	Engine::instance->sub_loop(this);
}
void FreeCam::update() {
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, position+forward, up);
	Camera::update();
}
void FreeCam::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(key==GLFW_KEY_ESCAPE&&action==GLFW_PRESS) {
		paused=!paused;
		Engine::instance->SetCursorMode(paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		return;
	}
	if(paused) return;
	if(key==GLFW_KEY_W) inputs[0]=action;
	else if(key==GLFW_KEY_A) inputs[1]=action;
	else if(key==GLFW_KEY_S) inputs[2]=action;
	else if(key==GLFW_KEY_D) inputs[3]=action;
	else if(key==GLFW_KEY_SPACE) inputs[4]=action;
	else if(key==GLFW_KEY_LEFT_SHIFT) inputs[5]=action;
}
void FreeCam::on_scroll(const double& xoffset, const double& yoffset) {
	fov-=static_cast<float>(yoffset);
	fov=clamp(fov, 1.0f, 90.0f);
	update();
}
void FreeCam::on_mouse_delta(const float& deltaX, const float& deltaY) {
	if(paused) return;
	pitch+=static_cast<float>(deltaY*SENSITIVITY);
	pitch=clamp(pitch, -89.0f, 89.0f);
	yaw+=static_cast<float>(deltaX*SENSITIVITY);
	Vector3 dir=Vector3(
		cos(deg_to_rad(pitch))*cos(deg_to_rad(yaw)),
		sin(deg_to_rad(pitch)),
		cos(deg_to_rad(pitch))*sin(deg_to_rad(yaw))
	).normalized();
	forward=dir;
	update();
}
void FreeCam::on_loop(const double& delta) {
	float deltaf=static_cast<float>(delta);
	Vector3 inputVec=Vector3(
		static_cast<float>(inputs[0]>=GLFW_PRESS)-static_cast<float>(inputs[2]>=GLFW_PRESS),
		static_cast<float>(inputs[4]>=GLFW_PRESS)-static_cast<float>(inputs[5]>=GLFW_PRESS),
		static_cast<float>(inputs[3]>=GLFW_PRESS)-static_cast<float>(inputs[1]>=GLFW_PRESS)
	);
	position+=((forward*inputVec.x)+(up*inputVec.y)+(forward.cross(up).normalized()*inputVec.z)).normalized()*SPEED*deltaf;
	update();
}
#pragma endregion// FreeCam
#pragma region OrthoCam
OrthoCam::OrthoCam(const Vector2& _position, const Vector2& _scale) :
	Camera(), hasTransform2D(_position), scale(_scale) {
	update();
}
void OrthoCam::update() {
	projection=ortho(-scale.x/2.0f, scale.x/2.0f, -scale.y/2.0f, scale.y/2.0f, 0.0f, 1000.0f);
	view=translate(Vector3(-getWorldPos(), 0));
}
#pragma endregion// OrthoCam