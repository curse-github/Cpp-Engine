#pragma once
#ifndef _ENGINE_H
#define _ENGINE_H

#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3
#include <vector>
#include <string>
#include <functional>
#ifdef _DEBUG
#define assert(expression, message) {\
if (!(expression)) {\
	std::cout << "Assertion failed: (" << message << ")\n" << __FILE__ << ", ln#" << __LINE__ << "\n";\
	std::cout << "Press enter to continue...\n";\
	std::cin.get();\
	abort();\
}\
}
#define assert_call(expression, message, codeBlock) {\
if (!(expression)) {\
	codeBlock;\
	std::cout << "Assertion failed: (" << message << ")\n" << __FILE__ << ", ln#" << __LINE__ << "\n";\
	std::cout << "Press enter to continue...\n";\
	std::cin.get();\
	abort();\
}\
}
#define engine_assert(expression, message) {\
if (!(expression)) {\
	Engine::instance->Delete();\
	delete Engine::instance;\
	std::cout << "Assertion failed: (" << message << ")\n" << __FILE__ << ", ln#" << __LINE__ << "\n";\
	std::cout << "Press enter to continue...\n";\
	std::cin.get();\
	abort();\
}\
}
#define engine_assert_call(expression, message, codeBlock) {\
if (!expression) {\
	codeBlock;\
	Engine::instance->Delete();\
	delete Engine::instance;\
	std::cout << "Assertion failed: (" << message << ")\n" << __FILE__ << ", ln#" << __LINE__ << "\n";\
	std::cout << "Press enter to continue...\n";\
	std::cin.get();\
	abort();\
}\
}
#else
#define assert(expression, message) {\
if (!(expression)) {\
	abort();\
}\
}
#define assert_call(expression, message, codeBlock) {\
if (!(expression)) {\
	codeBlock;\
	abort();\
}\
}
#define engine_assert(expression, message) {\
if (!(expression)) {\
	Engine::instance->Delete();\
	delete Engine::instance;\
	abort();\
}\
}
#define engine_assert_call(expression, message, codeBlock) {\
if (!(expression)) {\
	codeBlock;\
	Engine::instance->Delete();\
	delete Engine::instance;\
	abort();\
}\
}
#endif

#define PI 3.14159265f
#define TAU 6.2831853f

#include "EngineLib.h"

typedef std::function<void(const double&)> onloopfun;
void engine_on_error(int error, const char* description);
class Object;
//#define FpsAvgNum 60u
#define FpsAvgNum 1500u
class Engine {
	protected:
	void on_resize(int width, int height);
	void on_key(int key, int scancode, int action, int mods);
	void on_scroll(double xoffset, double yoffset);
	void on_mouse(double mouseX, double mouseY);
	void on_mouse_delta(float deltaX, float deltaY);
	void on_mouse_button(int button, int action, int mods);
	void on_mouse_enter(int entered);
	public:
	static Engine* instance;

	GLFWwindow* window=nullptr;
	std::vector<Object*> objects;
	bool ended=false;

	Vector2 curResolution;
	Vector2 curMousePos=Vector2(-1.0f, -1.0f);
	unsigned long frameCount=0u;
	unsigned int fpsAvg=0u;
	float frameTimeAvg=0u;
	unsigned int drawCalls=0u;
	unsigned int curDrawCalls=0u;

	double lastFpsTime=0.0f;// for calculating fps
	unsigned int fpsFrameCount=0u;// for calculating fps

	Engine() : window(nullptr), curResolution(Vector2()) {}
	Engine(const Vector2& size, const char* title, const bool& vsync);
	virtual ~Engine();
	Engine(Engine& copy)=delete;
	Engine(Engine&& move)=delete;
	void operator=(const Engine& other)=delete;

	void Start();
	protected:
	void Loop();
	public:
	void Close();
	void Delete();
	void SetCursorMode(const int& mode);

	std::vector<Object*> onResize;
	std::vector<Object*> onKey;
	std::vector<Object*> onScroll;
	std::vector<Object*> onMouse;
	std::vector<Object*> onMouseDelta;
	std::vector<Object*> onMouseButton;
	std::vector<Object*> onMouseEnter;
	std::vector<Object*> onStart;
	std::vector<Object*> onLoop;
	onloopfun renderLoop;

	void sub_resize(Object* obj);
	void sub_key(Object* obj);
	void sub_scroll(Object* obj);
	void sub_mouse(Object* obj);
	void sub_mouse_delta(Object* obj);
	void sub_mouse_button(Object* obj);
	void sub_mouse_enter(Object* obj);
	void sub_start(Object* obj);
	void sub_loop(Object* obj);

};
class Object {
	public:
	virtual void on_resize(const int& width, const int& height);
	virtual void on_key(const int& key, const int& scancode, const int& action, const int& mods);
	virtual void on_scroll(const double& xoffset, const double& yoffset);
	virtual void on_mouse(const double& mouseX, const double& mouseY);
	virtual void on_mouse_delta(const float& deltaX, const float& deltaY);
	virtual void on_mouse_button(const int& button, const int& action, const int& mods);
	virtual void on_mouse_enter(const int& entered);
	virtual void on_start();
	virtual void on_loop(const double& delta);
	Object();
	virtual ~Object();
};

struct Transform {
	public:
	Vector3 position;
	Vector3 scale;
	Vector3 rotAxis;
	float rotAngle;
	Transform() : position(Vector3::ZERO), scale(Vector3::ONE), rotAxis(Vector3::UP), rotAngle(0.0f) {};
	Transform(const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle);
};
struct Transform2D {
	private:
	Mat4x4 lastModelMat=Mat4x4();
	Vector2 lastWorldPosition=Vector2::ZERO;
	float lastZIndex=0.0f;
	Vector2 lastWorldScale=Vector2::ONE;
	float lastWorldRot=0.0f;
	protected:
	static bool AABBOverlap(const Vector2& aPos, const Vector2& aSize, const Vector2& bPos, const Vector2& bSize);
	public:
	Transform2D* parent=nullptr;
	std::vector<Transform2D*> children;
	Vector3 rotAxis=Vector3(0.0f, 0.0f, 1.0f);

	bool active;
	Vector2 position;
	float zIndex;
	Vector2 scale;
	Vector2 anchor;
	float rotAngle;

	Transform2D(const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f);
	virtual ~Transform2D();
	bool inRange(const Vector2& viewer, const Vector2& viewRange);
	bool inRange(const Vector2& viewer, const float& viewRange);

	bool isActive() const;
	Vector2 getWorldPos() const;
	Vector2 getWorldScale() const;
	float getWorldRot() const;
	void addChild(Transform2D* child);
	Mat4x4 getModelMat();

	void setWorldPos(Vector2 pos);
};
class hasTransform2D {
	public:
	Transform2D transform;
	hasTransform2D(const Vector2& _position=Vector2::ZERO, const float& _zIndex=0.0f, const Vector2& _scale=Vector2::ONE, const Vector2& _anchor=Vector2::Center, const float& _rotAngle=0.0f) : transform(Transform2D(_position, _zIndex, _scale, _anchor, _rotAngle)) {};
	bool inRange(const Vector2& viewer, const Vector2& viewRange) { return transform.inRange(viewer, viewRange); };
	bool inRange(const Vector2& viewer, const float& viewRange) { return transform.inRange(viewer, viewRange); };

	virtual bool isActive() const { return transform.isActive(); };
	virtual Vector2 getWorldPos() const { return transform.getWorldPos(); };
	virtual float getZIndex() const { return transform.zIndex; };
	virtual Vector2 getWorldScale() const { return transform.getWorldScale(); };
	virtual Vector2 getAnchor() const { return transform.anchor; };
	virtual float getWorldRot() const { return transform.getWorldRot(); };
	virtual void addChild(Transform2D* child) { return transform.addChild(child); };
	virtual void addChild(hasTransform2D* child) { return transform.addChild(&child->transform); };
	virtual Mat4x4 getModelMat() { return transform.getModelMat(); };

	void setWorldPos(Vector2 pos) { transform.setWorldPos(pos); };
};

class Texture;
class Shader : public Object {
	protected:
	unsigned int program=0;
	public:
	std::vector<Texture*> textures;
	std::vector<int> textureIndexes;
	unsigned int numTextures=0;
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	virtual ~Shader();
	void use();
	void setBool(const char* name, const bool& value);
	void setInt(const char* name, const int& value);
	void setFloat(const char* name, const float& value);
	void setFloat2(const char* name, const Vector2& value);
	void setFloat3(const char* name, const Vector3& value);
	void setFloat4(const char* name, const Vector4& value);
	void setMat4x4(const char* name, const Mat4x4& value);
	void setTexture(const char* name, Texture* tex, const unsigned int& location);
	void setTextureArray(const std::string& name);
	void bindTexture(const unsigned int& index);
	void bindTextures();
};
class Texture : public Object {
	public:
	unsigned int ID;
	int width;
	int height;
	Texture(const unsigned int& _ID);
	Texture(const std::string& path);
	void Bind(const unsigned int& location);
};

class VertexBufferObject;
class IndexBufferObject;

class VertexArrayObject {
	public:
	unsigned int VAO;
	VertexArrayObject(const bool& thing);
	~VertexArrayObject();
	void drawTris(const unsigned int& count);
	void drawTrisIndexed(const unsigned int& count);
	void drawTriStrip(const unsigned int& count);
	void drawTriStripIndexed(const unsigned int& count);
	void drawLines(const unsigned int& count, const float& width=1.0f, const bool& smooth=false);
	void drawLinesIndexed(const unsigned int& count, const float& width=1.0f, const bool& smooth=false);
	void drawLine(const unsigned int& count, const float& width=1.0f, const bool& loop=false, const bool& smooth=false);
	void drawLineIndexed(const unsigned int& count, const float& width=1.0f, const bool& loop=false, const bool& smooth=false);
};
class VertexBufferObject {
	public:
	unsigned int VBO;
	VertexArrayObject* VAO;
	VertexBufferObject(VertexArrayObject* _VAO);
	~VertexBufferObject();
	void staticFill(const float* vertices, const size_t& len);
	void staticFill(std::vector<float> vertices);
	void dynamicDefine(const size_t& len);
	void dynamicSub(const float* vertices, const size_t& len);
	void dynamicSub(const void* offset, const float* vertices, const size_t& len);
	void dynamicSub(std::vector<float> vertices);
	void applyAttributes(std::vector<unsigned int> attributes);
};
class IndexBufferObject {
	public:
	unsigned int EBO;// Element buffer object
	VertexArrayObject* VAO;
	IndexBufferObject(VertexArrayObject* _VAO);
	~IndexBufferObject();
	void staticFill(const unsigned int* indices, const size_t& len);
	void staticFill(std::vector<unsigned int> indices);
	void staticFillRepeated(const unsigned int* indices, const size_t& len, const size_t& count, const unsigned int& numIndices);
	void dynamicDefine(const size_t& len);
	void dynamicSub(const unsigned int* indices, const size_t& len);
	void dynamicSub(const void* offset, const unsigned int* indices, const size_t& len);
	void dynamicSub(std::vector<unsigned int> indices);
};

class StencilSimple {
	public:
	StencilSimple() {}
	void Enable();
	void Disable();
	void Write();
	void Compare();
};

#include "Cameras.h"
#include "Renderers.h"
//#include "BatchedRenderers.h"

#pragma region Shader Embedded Code
#define basicVertShader "#version 450 core\n\
layout (location = 0) in vec3 vecPos;\n\
layout (location = 1) in vec2 vecUV;\n\
uniform mat4 model;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
out vec2 uv;\n\
void main() {\n\
	gl_Position = projection*view*model*vec4(vecPos, 1.0);\n\
	uv = vecUV;\n\
}\0"
#define batchVertShader "#version 450 core\n\
layout (location = 0) in vec3 vecPos;\n\
layout (location = 1) in vec2 vecUV;\n\
layout (location = 2) in vec4 vecMod;\n\
layout (location = 3) in float vecTexIndex;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
out vec2 uv;\n\
out vec4 mod;\n\
out float texIndex;\n\
void main() {\n\
	gl_Position = projection*view*vec4(vecPos, 1.0);\n\
	uv=vecUV;\n\
	mod=vecMod;\n\
	texIndex=vecTexIndex;\n\
}\0"
#define colorFragShader "#version 450 core\n\
out vec4 outColor;\n\
in vec2 uv;\n\
uniform vec4 color;\n\
void main() {\n\
	outColor = color;\n\
}\0"
#define texFragShader "#version 450 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform sampler2D _texture;\n\
uniform vec4 modulate=vec4(1.0,1.0,1.0,1.0);\n\
void main() {\n\
	vec4 vertcolor = texture(_texture,uv);\n\
	if (vertcolor.a<0.05) { discard;return; }\n\
	if (modulate.r==0.0f&&modulate.g==0.0f&&modulate.b==0.0f&&modulate.a==0.0f) outColor = vertcolor;\n\
	else outColor = vertcolor*modulate;\n\
}\0"
#define textFragShader "#version 450 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform sampler2D text;\n\
uniform vec4 textColor;\n\
void main() {\n\
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, uv).r);\n\
	if (sampled.a<=0.05) { discard;return; }\n\
	outColor = textColor * sampled;\n\
}\0"
#define dotColorFragShader "#version 450 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform vec4 color;\n\
void main() {\n\
	if ((pow(uv.x-0.5f,2)+pow(uv.y-0.5f,2))>0.25f) { discard;return; }\n\
	else outColor = color;\n\
}\0"
#define dotTexFragShader "#version 450 core\n\
in vec2 uv;\n\
out vec4 outColor;\n\
uniform sampler2D _texture;\n\
uniform vec4 modulate=vec4(1.0,1.0,1.0,1.0);\n\
void main() {\n\
	if ((pow(uv.x-0.5f,2)+pow(uv.y-0.5f,2))>0.25f) { discard;return; }\n\
	vec4 vertcolor = texture(_texture,uv);\n\
	if (vertcolor.a<0.05) { discard;return; }\n\
	if (modulate.r==0.0f&&modulate.g==0.0f&&modulate.b==0.0f&&modulate.a==0.0f) outColor = vertcolor;\n\
	else outColor = vertcolor*modulate;\n\
}\0"
#define texBatchFragShader "#version 450 core\n\
in vec2 uv;\n\
in vec4 mod;\n\
in float texIndex;\n\
out vec4 outColor;\n\
uniform sampler2D _textures[32];\n\
void main() {\n\
	if (texIndex>=32||texIndex<0) outColor=mod;\n\
	else {\n\
		vec4 vertColor=texture(_textures[int(texIndex)],uv);\n\
		if (vertColor.a<=0.05f) { discard;return; }\n\
		if (mod.r==0.0f&&mod.g==0.0f&&mod.b==0.0f&&mod.a==0.0f) { outColor=vertColor; }\n\
		else { outColor=vertColor*mod; }\n\
	}\n\
}\0"
#define textBatchFragShader "#version 450 core\n\
in vec2 uv;\n\
in vec4 mod;\n\
in float texIndex;\n\
out vec4 outColor;\n\
uniform sampler2D _textures[32];\n\
void main() {\n\
	vec4 vertColor=vec4(1.0f,1.0f,1.0f,texture(_textures[int(texIndex)],uv).r);\n\
	if (vertColor.a<=0.05f) { discard;return; }\n\
	outColor=mod;\n\
}\0"
#define dotTexBatchFragShader "#version 450 core\n\
in vec2 uv;\n\
in vec4 mod;\n\
in float texIndex;\n\
out vec4 outColor;\n\
uniform sampler2D _textures[32];\n\
void main() {\n\
	if ((pow(uv.x-0.5f,2)+pow(uv.y-0.5f,2))>0.25f) { discard;return; }\n\
	if (texIndex>=32||texIndex<0) outColor=mod;\n\
	else {\n\
		vec4 vertColor=texture(_textures[int(texIndex)],uv);\n\
		if (vertColor.a<=0.05f) { discard;return; }\n\
		if (mod.r==0.0f&&mod.g==0.0f&&mod.b==0.0f&&mod.a==0.0f) { outColor=vertColor; }\n\
		else { outColor=vertColor*mod; }\n\
	}\n\
}\0"
#pragma endregion
#endif// _ENGINE_H