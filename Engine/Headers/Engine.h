#pragma once
#ifndef _ENGINEH
#define _ENGINEH
#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3
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
class Object;
class Engine {
public:
	GLFWwindow* window=nullptr;
	Vector2 screenSize;
	bool initialized=false;
	bool ended=false;
	Engine() : window(nullptr), screenSize(Vector2()) {}
	Engine(Vector2 size, const char* title, bool vsync);
	void Loop();
	void Close();
	void Delete();
	void SetCursor(int mode);

	std::vector<std::function<void(GLFWwindow*, int, int)>> onResize;
	std::vector<std::function<void(GLFWwindow*, int, int, int, int)>> onKey;
	std::vector<std::function<void(GLFWwindow*, double, double)>> onScroll;
	std::vector<std::function<void(GLFWwindow*, double, double)>> onMouse;
	std::vector<std::function<void(GLFWwindow*, float, float)>> onMouseDelta;
	std::vector<std::function<void(GLFWwindow*, int, int, int)>> onMouseButton;
	std::vector<std::function<void(GLFWwindow*, int)>> onMouseEnter;
	std::vector<ondeletefun> onDelete;
	std::vector<onloopfun> onLoop;

	void sub_resize(Object* obj);
	void sub_key(Object* obj);
	void sub_scroll(Object* obj);
	void sub_mouse(Object* obj);
	void sub_mouse_delta(Object* obj);
	void sub_mouse_button(Object* obj);
	void sub_mouse_enter(Object* obj);
	void sub_delete(Object* obj);
	void sub_loop(Object* obj);
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
public:
	Engine* engine;
	Object() : engine(nullptr) {}
	Object(Engine* _engine);


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
	std::vector<Texture*> textures;
	std::vector<int> textureIndexes;
public:
	Shader() : Object() {}
	Shader(Engine* _engine, std::string vertexPath, std::string fragmentPath);
	void use();
	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);
	void setFloat2(const std::string& name, Vector2 value);
	void setFloat3(const std::string& name, Vector3 value);
	void setFloat4(const std::string& name, Vector4 value);
	void setMat4x4(const std::string& name, Mat4x4 value);
	void setTexture(const std::string& name, Texture* tex, unsigned int location);
	void bindTextures();
};
class Camera : public Object {
protected:
	std::vector<Shader*> shaders;
public:
	Mat4x4 projection;
	Mat4x4 view;
	Camera() : Object(), projection(Mat4x4()), view(Mat4x4()) {}
	Camera(Engine* _engine);
	virtual void update();
	void bindShader(Shader* shader);
	void bindShaders(std::vector<Shader*> shaders);
	void use();
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
	std::string path;
	int width;
	int height;
	Texture() : Object(), ID(0), path(""), width(0), height(0) {}
	Texture(Engine* _engine, std::string _path);
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
class Renderer2D : public Renderer {
	public:
	bool hasBoundingBox;
	Renderer2D() : Renderer(), hasBoundingBox(false) {}
	Renderer2D(Engine* _engine, Shader* _shader) : Renderer(_engine,_shader), hasBoundingBox(false) {}
	Renderer2D(Engine* _engine, Shader* _shader, Vector2 _boundingBoxPos, Vector2 _boundingBoxSize) : Renderer(_engine,_shader), hasBoundingBox(true) {}
	virtual bool shouldDraw(Vector2 viewer, Vector2 viewRange);
	bool shouldDraw(Vector2 viewer, float viewRange);
};
class SpriteRenderer : public Renderer2D {
public:
	Vector2 position;
	Vector2 scale;
	Vector3 rotAxis=Vector3(0.0f, 0.0f, 1.0f);
	float zIndex=0;
	float rotAngle;
	SpriteRenderer() : Renderer2D(), position(Vector2()), scale(Vector2()), rotAngle(0.0f) {}
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale, float _zIndex);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale, float _zIndex, float _rotAngle);
	void draw() override;
	bool shouldDraw(Vector2 viewer, Vector2 viewRange) override;
};
extern bool characterMapInitialized;
class TextRenderer : public Renderer2D {
public:
	std::string text;
	Vector2 position;
	float scale;
	Vector3 color;
	TextRenderer() : Renderer2D(), text(""), position(Vector2()), scale(0), color(Vector3()) {}
	TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector2 _position, float _scale, Vector3 _color);
	void draw() override;
};
class LineRenderer : public Renderer2D {
	public:
	std::vector<Vector2> positions;
	float width;
	Vector2 position;
	Vector2 boundingBoxPos = Vector2(0.0f,0.0f);
	Vector2 boundingBoxSize = Vector2(0.0f,0.0f);
	bool loop;
	LineRenderer() : Renderer2D(), positions{}, width(1.0f), position(Vector2()), loop(false) {}
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position, bool _loop);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, bool _loop);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position);
	void draw() override;
	bool shouldDraw(Vector2 viewer, Vector2 viewRange) override;
};
class StencilSimple {
	public:
	StencilSimple() {}
	void Enable();
	void Disable();
	void Write();
	void Compare();
};

#define vsShader "#version 330 core\n\
layout(location=0) in vec3 vecPos;\n\
layout(location=1) in vec2 vecUV;\n\
uniform mat4 model;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
out vec2 uv;\n\
void main() {\n\
	gl_Position=projection * view * model * vec4(vecPos, 1.0);\n\
	uv=vecUV;\n\
}\0"
#define colorFragShader "#version 330 core\n\
out vec4 FragColor;\n\
in vec2 uv;\n\
uniform vec4 color;\n\
void main() {\n\
	FragColor = color;\n\
}\0"
#define texFragShader "#version 330 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform sampler2D _texture;\n\
uniform vec4 modulate=vec4(1.0, 1.0, 1.0, 1.0);\n\
void main() {\n\
	vec4 vertcolor=texture(_texture, uv);\n\
	if(vertcolor.a < 0.05) discard;\n\
	if(modulate.r == 0 && modulate.g == 0 && modulate.b == 0 && modulate.a == 0) outColor=vertcolor;\n\
	else outColor=vertcolor * modulate;\n\
}\0"
#define textVsShader "#version 330 core\n\
layout(location=0) in vec3 vecPos;\n\
layout(location=1) in vec2 vecUV;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
out vec2 uv;\n\
void main() {\n\
	gl_Position=projection * view * vec4(vecPos, 1.0);\n\
	uv=vecUV;\n\
}\0"
#define textFragShader "#version 330 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform sampler2D text;\n\
uniform vec3 textColor;\n\
void main() {\n\
	vec4 sampled=vec4(1.0, 1.0, 1.0, texture(text, uv).r); \n\
	if(sampled.a <= 0.05) discard; \n\
	outColor=vec4(textColor, 1.0) * sampled; \n\
}\0"
#endif