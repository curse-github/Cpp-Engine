#pragma once
#ifndef _ENGINEH
#define _ENGINEH
#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#include <vector>
#include <string>
#include <functional>
#include "Lib.h"
typedef std::function<void(GLFWwindow*, int, int)>           onresizefun;
typedef std::function<void(GLFWwindow*, int, int, int, int)> onkeyfun;
typedef std::function<void(GLFWwindow*, double, double)>     onscrollfun;
typedef std::function<void(GLFWwindow*, double, double)>     onmousefun;
typedef std::function<void(GLFWwindow*, float, float)>       onmousedeltafun;
typedef std::function<void(GLFWwindow*, int, int, int)>      onmousebuttonfun;
typedef std::function<void(GLFWwindow*, int)>                onmouseenterfun;
typedef std::function<void()>                                ondeletefun;
typedef std::function<void(double)>                          onloopfun;

void engine_on_error(int error, const char* description);
class Engine {
	public:
	GLFWwindow* window=nullptr;
	Vector2 screenSize;
	bool initialized=false;
	bool ended = false;
	Engine() {}
	Engine(Vector2 size, const char* title, bool vsync);
	void Loop();
	void Close();
	void Delete();

	std::vector<onresizefun> onResize;
	std::vector<onkeyfun> onKey;
	std::vector<onscrollfun> onScroll;
	std::vector<onmousefun> onMouse;
	std::vector<onmousedeltafun> onMouseDelta;
	std::vector<onmousebuttonfun> onMouseButton;
	std::vector<onmouseenterfun> onMouseEnter;
	std::vector<ondeletefun> onDelete;
	std::vector<onloopfun> onLoop;
	protected:
	void on_resize(GLFWwindow* window, int width, int height);
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
	void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
	Vector2 lastMouse;
	void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_enter(GLFWwindow* window, int entered);
};
class Object {
	public:
	bool initialized=false;
	protected:
	Engine engine;
	Object() : engine(Engine()) {}
	Object(Engine _engine);

	void sub_resize();
	void sub_key();
	void sub_scroll();
	void sub_mouse();
	void sub_mouse_delta();
	void sub_mouse_button();
	void sub_mouse_enter();
	void sub_delete();
	void sub_loop();

	void on_resize(GLFWwindow* window, int width, int height);
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
	void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
	void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_enter(GLFWwindow* window, int entered);
	void on_delete();
	void on_loop(double delta);
};
class Shader : public Object {
	protected:
	unsigned int program=0;
	void on_delete();
	public:
	Shader() : Object() {}
	Shader(Engine _engine, string vertexPath, string fragmentPath);
	void use();
	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);
	void setFloat2(const std::string& name, Vector2 value);
	void setFloat3(const std::string& name, Vector3 value);
	void setFloat4(const std::string& name, Vector4 value);
	void setMat4x4(const std::string& name, Mat4x4 value);
};
class Camera : public Object {
	public:
	Shader shader;
	Mat4x4 projection;
	Mat4x4 view;
	Camera() : Object(), shader(), projection(Mat4x4()), view(Mat4x4()) {}
	Camera(Engine _engine, Shader _shader);
	void updateProjection();
	void updateView();
};
class LookAtCam : public Camera {
	public:
	float fov=45;
	float aspect;
	Vector3 position;
	Vector3 focus;
	LookAtCam() : Camera(), position(Vector3()), aspect(0.0f), focus(Vector3()) {}
	LookAtCam(Engine _engine, Shader _shader, float _aspect, Vector3 _position, Vector3 _focus);
	void updateProjection();
	void updateView();
};
class CubeRenderer : public Object {
	protected:
	Shader shader;
	unsigned int VAO;
	unsigned int VBO;
	void on_delete();
	public:
	Vector3 position;
	Vector3 rotAxis;
	float rotAngle;
	CubeRenderer() : Object(), shader(), position(Vector3()), rotAxis(Vector3()), rotAngle(0.0f), VAO(0), VBO(0) {}
	CubeRenderer(Engine _engine, Shader _shader, Vector3 _position, Vector3 _rotAxis, float _rotAngle);
	void draw();
};
#endif