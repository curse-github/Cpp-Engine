#pragma once
#ifndef _ENGINEH
#define _ENGINEH
#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3
#include <vector>
#include <string>
#include <functional>
#include "Lib.h"
#include <map>
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
	bool ended=false;
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
	Vector2 lastMouse=Vector2(-1.0f, -1.0f);
	void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_enter(GLFWwindow* window, int entered);
};
class Object {
	public:
	bool initialized=false;
	protected:
	Engine* engine;
	Object() : engine(nullptr) {}
	Object(Engine* _engine);

	void sub_resize();
	void sub_key();
	void sub_scroll();
	void sub_mouse();
	void sub_mouse_delta();
	void sub_mouse_button();
	void sub_mouse_enter();
	void sub_delete();
	void sub_loop();

	virtual void on_resize(GLFWwindow* window, int width, int height);
	virtual void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
	virtual void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
	virtual void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	virtual void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	virtual void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	virtual void on_mouse_enter(GLFWwindow* window, int entered);
	virtual void on_delete();
	virtual void on_loop(double delta);
};
class Texture;
class Shader : public Object {
	protected:
	unsigned int program=0;
	void on_delete() override;
	std::vector<Texture> textures;
	std::vector<unsigned int> textureLocations;
	public:
	Shader() : Object() {}
	Shader(Engine* _engine, string vertexPath, string fragmentPath);
	void use();
	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);
	void setFloat2(const std::string& name, Vector2 value);
	void setFloat3(const std::string& name, Vector3 value);
	void setFloat4(const std::string& name, Vector4 value);
	void setMat4x4(const std::string& name, Mat4x4 value);
	void setTexture(const std::string& name, Texture tex, unsigned int location);
	void bindTextures();
};
class Camera : public Object {
	protected:
	Camera* self;
	public:
	Mat4x4 projection;
	Mat4x4 view;
	Camera() : Object(), self(nullptr), projection(Mat4x4()), view(Mat4x4()) {}
	Camera(Engine* _engine);
	virtual void update();
	void set(Shader* shader);
};
class LookAtCam : public Camera {
	public:
	float fov=45;
	float aspect;
	Vector3 position;
	Vector3 focus;
	LookAtCam() : Camera(), aspect(0.0f), position(Vector3()), focus(Vector3()) {}
	LookAtCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _focus);
	void update();
};
class FreeCam : public Camera {
	protected:
	float aspect;
	Vector3 position;
	Vector3 forward;
	Vector3 up;
	float SPEED=2.5f;
	float pitch=0.0f;
	float yaw=-90.0f;
	public:
	float fov=45;
	float SENSITIVITY=0.1f;
	bool paused=false;
	FreeCam() : Camera(), aspect(0.0f), position(Vector3()), forward(Vector3()), up(Vector3()) {}
	FreeCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _forward, Vector3 _up);
	void update();
	void on_loop(double delta) override;
	int inputs[6]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) override;
	void on_scroll(GLFWwindow* window, double xoffset, double yoffset) override;
};
class OrthoCam : public Camera {
	public:
	Vector2 position;
	Vector2 size;
	OrthoCam() : Camera(), position(Vector2()), size(Vector2()) {}
	OrthoCam(Engine* _engine, Vector2 _position, Vector2 _size);
	void update();
};
class Texture : public Object {
	public:
	unsigned int ID;
	public:
	string path;
	Texture();
	Texture(Engine* _engine, string _path);
	void Bind(Shader* shader, unsigned int location);
};
class Renderer : public Object {
	protected:
	Shader* shader;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	void on_delete() override;
	public:
	Renderer() : Object(), shader(nullptr), VAO(0), VBO(0), EBO(0) {}
	Renderer(Engine* _engine, Shader* _shader);
	virtual void draw();
};
class CubeRenderer : public Renderer {
	public:
	Vector3 position;
	Vector3 rotAxis;
	float rotAngle;
	CubeRenderer() : Renderer(), position(Vector3()), rotAxis(Vector3()), rotAngle(0.0f) {}
	CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _rotAxis, float _rotAngle);
	void draw() override;
};
class SpriteRenderer : public Renderer {
	public:
	Vector2 position;
	Vector3 rotAxis=Vector3(0.0f, 0.0f, 1.0f);
	float rotAngle;
	SpriteRenderer() : Renderer(), position(Vector2()), rotAngle(0.0f) {}
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _rotAngle);
	void draw() override;
};
class TextRenderer : public Renderer {
	public:
	std::string text;
	Vector2 position;
	float scale;
	Vector3 color;
	TextRenderer() : Renderer(), text(""), position(Vector2()), scale(0), color(Vector3()) {}
	TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector2 _position, float _scale, Vector3 _color);
	void draw() override;
};
#endif