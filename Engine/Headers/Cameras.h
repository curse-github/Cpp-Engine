#pragma once
#ifndef _CAMERA_H
#define _CAMERA_H

#include "Engine.h"

class Camera : public Object {
	std::vector<Shader*> shaders;
	public:
	Mat4x4 projection;
	Mat4x4 view;
	Camera() : Object(), projection(Mat4x4()), view(Mat4x4()) {}
	Camera(Engine* _engine);
	virtual void update();
	void bindShader(Shader* shader);
	void bindShaders(const std::vector<Shader*>& shaders);
	void use();
};
class LookAtCam : public Camera, public Transform {
	public:
	float fov=45;
	float aspect;
	Vector3 focus;
	LookAtCam() : Camera(), Transform(), aspect(0.0f), focus(Vector3()) {}
	LookAtCam(Engine* _engine, const float& _aspect, const Vector3& _position, const Vector3& _focus);
	void update();
};
class FreeCam : public Camera, public Transform {
	protected:
	float aspect;
	Vector3 forward;
	Vector3 up;
	float SPEED=2.5f;
	float pitch=0.0f;
	float yaw=-90.0f;
	void on_loop(const double& delta) override;
	int inputs[6]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, const int& key, const int& scancode, const int& action, const int& mods) override;
	void on_mouse_delta(GLFWwindow* window, const float& deltaX, const float& deltaY) override;
	void on_scroll(GLFWwindow* window, const double& xoffset, const double& yoffset) override;
	public:
	float fov=45;
	float SENSITIVITY=0.1f;
	bool paused=false;
	FreeCam() : Camera(), Transform(), aspect(0.0f), forward(Vector3()), up(Vector3()) {}
	FreeCam(Engine* _engine, const float& _aspect, const Vector3& _position, const Vector3& _forward, const Vector3& _up);
	void update();
};
class OrthoCam : public Camera, public Transform2D {
	public:
	OrthoCam() : Camera(), Transform2D() {}
	OrthoCam(Engine* _engine, const Vector2& _position, const Vector2& _scale);
	void update();
};

#endif// _CAMERA_H