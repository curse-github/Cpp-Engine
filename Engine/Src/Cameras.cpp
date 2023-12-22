#include "Cameras.h"

#define clamp(a,b,c) std::max(b,std::min(a,c))

#pragma region Camera
Camera::Camera(Engine* _engine) : Object(_engine), projection(Mat4x4()), view(Mat4x4()) {
	if(!initialized) return;
}
void Camera::update() {}
void Camera::bindShader(Shader* shader) {
	if(engine->ended||!initialized||!shader->initialized) return;
	shaders.push_back(shader);
}
void Camera::bindShaders(const std::vector<Shader*>& shaders) {
	for(Shader* shdr:shaders) bindShader(shdr);
}
void Camera::use() {
	if(engine->ended||!initialized) return;
	for(Shader* shdr:shaders) {
		if(!shdr->initialized) continue;
		shdr->setMat4x4("projection", projection);
		shdr->setMat4x4("view", view);
	}
}
#pragma endregion// Camera
#pragma region LookAtCam
LookAtCam::LookAtCam(Engine* _engine, const float& _aspect, const Vector3& _position, const Vector3& _focus) :
	Camera(_engine), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), focus(_focus) {
	if(!initialized) return;
	update();
}
void LookAtCam::update() {
	if(engine->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}
#pragma endregion// LookAtCam
#pragma region FreeCam
FreeCam::FreeCam(Engine* _engine, const float& _aspect, const Vector3& _position, const Vector3& _forward, const Vector3& _up) :
	Camera(_engine), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), forward(_forward), up(_up) {
	if(!initialized) return;
	glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	engine->sub_key(this);
	engine->sub_scroll(this);
	engine->sub_mouse_delta(this);
	engine->sub_loop(this);
}
void FreeCam::update() {
	if(engine->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, position+forward, up);
	Camera::update();
}
void FreeCam::on_key(GLFWwindow* window, const int& key, const int& scancode, const int& action, const int& mods) {
	if(engine->ended||!initialized) return;
	if(key==GLFW_KEY_ESCAPE&&action==GLFW_PRESS) {
		paused=!paused;
		engine->SetCursorMode(paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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
void FreeCam::on_scroll(GLFWwindow* window, const double& xoffset, const double& yoffset) {
	if(engine->ended||!initialized) return;
	fov-=(float)yoffset;
	fov=clamp(fov, 1.0f, 90.0f);
	update();
}
void FreeCam::on_mouse_delta(GLFWwindow* window, const float& deltaX, const float& deltaY) {
	if(engine->ended||!initialized) return;
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
	if(engine->ended||!initialized) return;
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
OrthoCam::OrthoCam(Engine* _engine, const Vector2& _position, const Vector2& _scale) :
	Camera(_engine), Transform2D(_position, 0.0f, _scale, Vector2(), 0.0f) {
	if(!initialized) return;
	update();
}
void OrthoCam::update() {
	if(engine->ended||!initialized) return;
	projection=ortho(-scale.x/2.0f, scale.x/2.0f, -scale.y/2.0f, scale.y/2.0f, 0.0f, 1000.0f);
	view=translate(Vector3(-getWorldPos(), 0));
}
#pragma endregion// OrthoCam