#include "Cameras.h"

#define clamp(a,b,c) std::max(b,std::min(a,c))

#pragma region Camera
Camera::Camera() : Object(), projection(Mat4x4()), view(Mat4x4()) {
	if(!initialized) return;
}
void Camera::update() {}
void Camera::bindShader(Shader* shader) {
	if(Engine::instance->ended||!initialized||!shader->initialized) return;
	shaders.push_back(shader);
}
void Camera::bindShaders(const std::vector<Shader*>& shaders) {
	for(Shader* shdr:shaders) bindShader(shdr);
}
void Camera::use() {
	if(Engine::instance->ended||!initialized) return;
	for(Shader* shdr:shaders) {
		if(!shdr->initialized) continue;
		shdr->setMat4x4("projection", projection);
		shdr->setMat4x4("view", view);
	}
}
#pragma endregion// Camera
#pragma region LookAtCam
LookAtCam::LookAtCam(const float& _aspect, const Vector3& _position, const Vector3& _focus) :
	Camera(), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), focus(_focus) {
	if(!initialized) return;
	update();
}
void LookAtCam::update() {
	if(Engine::instance->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}
#pragma endregion// LookAtCam
#pragma region FreeCam
FreeCam::FreeCam(const float& _aspect, const Vector3& _position, const Vector3& _forward, const Vector3& _up) :
	Camera(), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), forward(_forward), up(_up) {
	if(!initialized) return;
	Engine::instance->SetCursorMode(GLFW_CURSOR_DISABLED);
	Engine::instance->sub_key(this);
	Engine::instance->sub_scroll(this);
	Engine::instance->sub_mouse_delta(this);
	Engine::instance->sub_loop(this);
}
void FreeCam::update() {
	if(Engine::instance->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, position+forward, up);
	Camera::update();
}
void FreeCam::on_key(const int& key, const int& scancode, const int& action, const int& mods) {
	if(Engine::instance->ended||!initialized) return;
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
	if(Engine::instance->ended||!initialized) return;
	fov-=(float)yoffset;
	fov=clamp(fov, 1.0f, 90.0f);
	update();
}
void FreeCam::on_mouse_delta(const float& deltaX, const float& deltaY) {
	if(Engine::instance->ended||!initialized) return;
	if(paused) return;
	pitch+=(float)deltaY*SENSITIVITY;
	pitch=clamp(pitch, -89.0f, 89.0f);
	yaw+=(float)deltaX*SENSITIVITY;
	Vector3 dir=Vector3(
		cos(deg_to_rad(pitch))*cos(deg_to_rad(yaw)),
		sin(deg_to_rad(pitch)),
		cos(deg_to_rad(pitch))*sin(deg_to_rad(yaw))
	).normalized();
	forward=dir;
	update();
}
void FreeCam::on_loop(const double& delta) {
	if(Engine::instance->ended||!initialized) return;
	float deltaf=((float)delta);
	Vector3 inputVec=Vector3(
		(float)(inputs[0]>=GLFW_PRESS)-(float)(inputs[2]>=GLFW_PRESS),
		(float)(inputs[4]>=GLFW_PRESS)-(float)(inputs[5]>=GLFW_PRESS),
		(float)(inputs[3]>=GLFW_PRESS)-(float)(inputs[1]>=GLFW_PRESS)
	);
	position+=((forward*inputVec.x)+(up*inputVec.y)+(forward.cross(up).normalized()*inputVec.z)).normalized()*SPEED*deltaf;
	update();
}
#pragma endregion// FreeCam
#pragma region OrthoCam
OrthoCam::OrthoCam(const Vector2& _position, const Vector2& _scale) :
	Camera(), hasTransform2D(_position), scale(_scale) {
	if(!initialized) return;
	update();
}
void OrthoCam::update() {
	if(Engine::instance->ended||!initialized) return;
	projection=ortho(-scale.x/2.0f, scale.x/2.0f, -scale.y/2.0f, scale.y/2.0f, 0.0f, 1000.0f);
	view=translate(Vector3(-getWorldPos(), 0));
}
#pragma endregion// OrthoCam