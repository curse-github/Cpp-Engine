#pragma once
#ifndef _ENGINEH
#define _ENGINEH
#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3
#include <vector>
#include <string>
#include <functional>
#include "Lib.h"
typedef std::function<void(double)> onloopfun;

void engine_on_error(int error, const char* description);
class Object;
class Engine {
	protected:
	void on_resize(GLFWwindow* window, int width, int height);
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
	void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
	Vector2 lastMouse=Vector2(-1.0f, -1.0f);
	void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_enter(GLFWwindow* window, int entered);
	public:
	GLFWwindow* window=nullptr;
	Vector2 screenSize;
	bool initialized=false;
	bool ended=false;
	std::vector<Object*> objects;
	Engine() : window(nullptr), screenSize(Vector2()) {}
	Engine(Vector2 size, const char* title, bool vsync);
	virtual ~Engine();
	void Loop();
	void Close();
	void Delete();
	void SetCursorMode(int mode);

	std::vector<Object*> onResize;
	std::vector<Object*> onKey;
	std::vector<Object*> onScroll;
	std::vector<Object*> onMouse;
	std::vector<Object*> onMouseDelta;
	std::vector<Object*> onMouseButton;
	std::vector<Object*> onMouseEnter;
	std::vector<Object*> onLoop;
	onloopfun renderLoop;

	void sub_resize(Object* obj);
	void sub_key(Object* obj);
	void sub_scroll(Object* obj);
	void sub_mouse(Object* obj);
	void sub_mouse_delta(Object* obj);
	void sub_mouse_button(Object* obj);
	void sub_mouse_enter(Object* obj);
	void sub_loop(Object* obj);
};
class Object {
	protected:
	Engine* engine;
	friend Engine;
	public:
	virtual void on_resize(GLFWwindow* window, int width, int height);
	virtual void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
	virtual void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
	virtual void on_mouse(GLFWwindow* window, double mouseX, double mouseY);
	virtual void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY);
	virtual void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	virtual void on_mouse_enter(GLFWwindow* window, int entered);
	virtual void on_loop(double delta);
	bool initialized=false;
	Object() : engine(nullptr) {}
	Object(Engine* _engine);
	virtual ~Object();
};
class Transform {
	public:
	Vector3 position;
	Vector3 scale;
	Vector3 rotAxis;
	float rotAngle;
	Transform() : position(Vector3()), scale(Vector3()), rotAxis(Vector3()), rotAngle(0.0f) {};
	Transform(Vector3 _position, Vector3 _scale, Vector3 _rotAxis, float _rotAngle);
};
class Transform2D {
	public:
	Vector2 position;
	float zIndex;
	Vector2 scale;
	Vector2 anchor;
	Vector3 rotAxis=Vector3(0.0f, 0.0f, 1.0f);
	float rotAngle;
	Transform2D() : position(Vector2()), zIndex(0.0f), scale(Vector2()), anchor(Vector2()), rotAngle(0.0f) {};
	Transform2D(Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle);
	Mat4x4 getTranslateMat();
	Mat4x4 getModel();
};
class Texture;
class Shader : public Object {
	protected:
	unsigned int program=0;
	std::vector<Texture*> textures;
	std::vector<int> textureIndexes;
	public:
	Shader() : Object() {}
	Shader(Engine* _engine, std::string vertexPath, std::string fragmentPath);
	virtual ~Shader();
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
class LookAtCam : public Camera, protected Transform {
	public:
	using Transform::position;
	float fov=45;
	float aspect;
	Vector3 focus;
	LookAtCam() : Camera(), Transform(), aspect(0.0f), focus(Vector3()) {}
	LookAtCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _focus);
	void update();
};
class FreeCam : public Camera, protected Transform {
	protected:
	float aspect;
	using Transform::position;
	Vector3 forward;
	Vector3 up;
	float SPEED=2.5f;
	float pitch=0.0f;
	float yaw=-90.0f;
	void on_loop(double delta) override;
	int inputs[6]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) override;
	void on_scroll(GLFWwindow* window, double xoffset, double yoffset) override;
	public:
	float fov=45;
	float SENSITIVITY=0.1f;
	bool paused=false;
	FreeCam() : Camera(), Transform(), aspect(0.0f), forward(Vector3()), up(Vector3()) {}
	FreeCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _forward, Vector3 _up);
	void update();
};
class OrthoCam : public Camera, protected Transform2D {
	public:
	using Transform2D::position;
	using Transform2D::scale;
	OrthoCam() : Camera(), Transform2D() {}
	OrthoCam(Engine* _engine, Vector2 _position, Vector2 _size);
	void update();
};
class Texture : public Object {
	protected:
	unsigned int ID;
	std::string path;
	public:
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
	public:
	Renderer() : Object(), shader(nullptr), VAO(0), VBO(0), EBO(0) {}
	Renderer(Engine* _engine, Shader* _shader);
	virtual ~Renderer();
	void setShader(Shader* _shader);
	virtual void draw()=0;
};
class CubeRenderer : public Renderer, public Transform {
	public:
	CubeRenderer() : Renderer(), Transform() {}
	CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _scale, Vector3 _rotAxis, float _rotAngle);
	CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _scale);
	void draw() override;
};
class Renderer2D : public Renderer, public Transform2D {
	protected:
	static bool AABBOverlap(Vector2 aPos, Vector2 aSize, Vector2 bPos, Vector2 bSize);
	public:
	Renderer2D() : Renderer(), Transform2D() {};
	Renderer2D(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle) :
		Renderer(_engine, _shader), Transform2D(_position, _zIndex, _scale, _anchor, _rotAngle) {};
	bool shouldDraw(Vector2 viewer, Vector2 viewRange);
	bool shouldDraw(Vector2 viewer, float viewRange);
	bool shouldDraw(OrthoCam* viewer);
};
class SpriteRenderer : public Renderer2D {
	public:
	SpriteRenderer() : Renderer2D() {}
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex);
	SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position);
	void draw() override;
	using Renderer2D::shouldDraw;
};
extern bool characterMapInitialized;
class TextRenderer : protected Renderer2D {
	public:
	std::string text;
	Vector3 color;
	using Transform2D::position;
	using Transform2D::zIndex;
	float scale;
	using Transform2D::anchor;
	TextRenderer() : Renderer2D(), text(""), color(Vector3()), scale(0.0f) {}
	TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale, float _zIndex, Vector2 _anchor);
	TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale, float _zIndex);
	TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale);
	void draw() override;
	using Renderer2D::shouldDraw;
};
class LineRenderer : protected Renderer2D {
	protected:
	std::vector<Vector2> positions;
	public:
	using Transform2D::position;
	float width;
	bool loop;
	LineRenderer() : Renderer2D(), positions {}, width(1.0f), loop(false) {}
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position, bool _loop);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, bool _loop);
	LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width);
	void draw() override;
	using Renderer2D::shouldDraw;
};
class StencilSimple {
	public:
	StencilSimple() {}
	void Enable();
	void Disable();
	void Write();
	void Compare();
};

#pragma region Shader Embedded Code
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
#pragma endregion
#endif