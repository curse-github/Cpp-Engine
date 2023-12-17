#include "Engine.h"
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include <freetype/freetype.h>
#include <map>
#include <limits>
#define clamp(a,b,c) std::max(b,std::min(a,c))

#pragma region Engine
void engine_on_error(int error, const char* description) {
	Log("GLDW error: "+std::string(description));
#ifdef _DEBUG
	__debugbreak();
#endif
}

Engine::Engine(Vector2 size, const char* title, bool vsync) : window(nullptr), screenSize(size) {
	if(glfwInit()==GLFW_FALSE) {
		ended=true;
		Log("GLFW failed to init.");//error
		return;
	}

	window=glfwCreateWindow((int)size.x, (int)size.y, title, NULL, NULL);
	if(!window) {
		ended=true;
		glfwTerminate();
		Log("Window failed to create.");//error
		return;
	}
	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ended=true;
		glfwDestroyWindow(window);
		glfwTerminate();
		Log("GLAD failed to init.");//error
		return;
	}
	glViewport(0, 0, (int)size.x, (int)size.y);
	glfwSwapInterval((int)vsync);// V-Sync: 1=on, 0=off

	glfwSetWindowUserPointer(window, this);
	glfwSetErrorCallback(engine_on_error);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_resize(window, width, height); });
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_key(window, key, scancode, action, mods); });
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_scroll(window, xoffset, yoffset); });
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double mouseX, double mouseY) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse(window, mouseX, mouseY); });
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse_button(window, button, action, mods); });
	glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse_enter(window, entered); });

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	initialized=true;
}
Engine::~Engine() {
	if(!initialized) return;
	//Todo: upper bound?
	while(objects.size()>0) {
		Object* ptr=objects[0];
		if(ptr) delete ptr;
	}
}
void Engine::Loop() {
	if(ended||!initialized) return;
	double lastFrameTime=glfwGetTime();
	while(!glfwWindowShouldClose(window)) {
		double delta=glfwGetTime()-lastFrameTime;
		lastFrameTime=glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		for(unsigned int i=0; i<onLoop.size(); i++) {
			onLoop[i]->on_loop(delta);
		}
		renderLoop(delta);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	Delete();
}
void Engine::Close() {
	if(ended||!initialized) return;
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void Engine::Delete() {
	if(ended||!initialized) return;
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	ended=true;
	glfwDestroyWindow(window);
	glfwTerminate();
}
void Engine::SetCursorMode(int mode) {
	if(ended||!initialized) return;
	glfwSetInputMode(window, GLFW_CURSOR, mode);
	lastMouse=Vector2(-1.0f, -1.0f);
}

#pragma region callbacks
void Engine::on_resize(GLFWwindow* window, int width, int height) {
	if(ended||!initialized) return;
	glViewport(0, 0, width, height); screenSize=Vector2((float)width, (float)height);
	for_each(onResize.begin(), onResize.end(), [window, width, height](Object* obj) {
		obj->on_resize(window, width, height);
		});
}
void Engine::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(ended||!initialized) return;
	for_each(onKey.begin(), onKey.end(), [window, key, scancode, action, mods](Object* obj) {
		obj->on_key(window, key, scancode, action, mods);
		});
}
void Engine::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	if(ended||!initialized) return;
	for_each(onScroll.begin(), onScroll.end(), [window, xoffset, yoffset](Object* obj) {
		obj->on_scroll(window, xoffset, yoffset);
		});
}
void Engine::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {
	if(ended||!initialized) return;
	if(lastMouse.x==-1||lastMouse.y==-1) {
		lastMouse.x=(float)mouseX; lastMouse.y=(float)mouseY; return;
	}
	float deltaX=((float)mouseX)-lastMouse.x;
	float deltaY=lastMouse.y-((float)mouseY);
	lastMouse=Vector2((float)mouseX, (float)mouseY);
	for_each(onMouse.begin(), onMouse.end(), [window, mouseX, mouseY](Object* obj) {
		obj->on_mouse(window, mouseX, mouseY);
		});
	if((deltaX!=0)||(deltaY!=0)) on_mouse_delta(window, deltaX, deltaY);
}
void Engine::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {
	if(ended||!initialized) return;
	for_each(onMouseDelta.begin(), onMouseDelta.end(), [window, deltaX, deltaY](Object* obj) {
		obj->on_mouse_delta(window, deltaX, deltaY);
		});
}
void Engine::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	if(ended||!initialized) return;
	for_each(onMouseButton.begin(), onMouseButton.end(), [window, button, action, mods](Object* obj) {
		obj->on_mouse_button(window, button, action, mods);
		});
}
void Engine::on_mouse_enter(GLFWwindow* window, int entered) {
	if(ended||!initialized) return;
	for_each(onMouseEnter.begin(), onMouseEnter.end(), [window, entered](Object* obj) {
		obj->on_mouse_enter(window, entered);
		});
}
#pragma endregion// callbacks

#pragma region subFuncs
void Engine::sub_resize(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onResize.push_back(obj);
}
void Engine::sub_key(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onKey.push_back(obj);
}
void Engine::sub_scroll(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onScroll.push_back(obj);
}
void Engine::sub_mouse(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouse.push_back(obj);
}
void Engine::sub_mouse_delta(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseDelta.push_back(obj);
}
void Engine::sub_mouse_button(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseButton.push_back(obj);
}
void Engine::sub_mouse_enter(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseEnter.push_back(obj);
}
void Engine::sub_loop(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onLoop.push_back(obj);
}
#pragma endregion// subFuncs

#pragma endregion// Engine
#pragma region Object
Object::Object(Engine* _engine) : engine(_engine) {
	if(!engine->initialized||engine->ended) { initialized=false; return; }
	initialized=true;
	engine->objects.push_back(this);
}
template <typename T> void vecRemoveValue(std::vector<T>& vec, const T& value) {
	for(unsigned int i=0; i<vec.size(); i++) {
		if(vec[i]==value) vec.erase(vec.begin()+i);
	}
}
Object::~Object() {
	vecRemoveValue(engine->objects, this);
	vecRemoveValue(engine->onResize, this);
	vecRemoveValue(engine->onKey, this);
	vecRemoveValue(engine->onScroll, this);
	vecRemoveValue(engine->onMouse, this);
	vecRemoveValue(engine->onMouseDelta, this);
	vecRemoveValue(engine->onMouseButton, this);
	vecRemoveValue(engine->onMouseEnter, this);
	vecRemoveValue(engine->onLoop, this);
}

void Object::on_resize(GLFWwindow* window, int width, int height) {}
void Object::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {}
void Object::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {}
void Object::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {}
void Object::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {}
void Object::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {}
void Object::on_mouse_enter(GLFWwindow* window, int entered) {}
void Object::on_loop(double delta) {}
#pragma endregion// Object

#pragma region Transforms
Transform::Transform(Vector3 _position, Vector3 _scale, Vector3 _rotAxis, float _rotAngle) :
	position(_position), scale(_scale), rotAxis(_rotAxis), rotAngle(_rotAngle) {}
Transform2D::Transform2D(Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle) :
	position(_position), zIndex(_zIndex), scale(_scale), anchor(_anchor), rotAngle(_rotAngle) {
	lastModelMat=translate(Vector3(-anchor, 0.0f))*axisRotMat(rotAxis, deg_to_rad(rotAngle))*scaleMat(Vector3(scale, 1.0f))*translate(Vector3(position, zIndex-100.0f));
}
Mat4x4 Transform2D::createModelMat(Vector2 _position=Vector2::ZERO, float _zIndex=0.0f, Vector2 _scale=Vector2::ONE, Vector2 _anchor=Vector2::Center, float _rotAngle=0.0f) {
	return translate(Vector3(-_anchor, 0.0f))*axisRotMat(Vector3(0.0f, 0.0f, 1.0f), deg_to_rad(_rotAngle))*scaleMat(Vector3(_scale, 1.0f))*translate(Vector3(_position, _zIndex-100.0f));
}
Mat4x4 Transform2D::getModelMat() {
	bool changed=false;
	if(lastPosition!=position) { lastPosition=position;changed=true; }
	if(lastZIndex!=zIndex) { lastZIndex=zIndex; }
	if(lastScale!=scale) { lastScale=scale; }
	if(lastAnchor!=anchor) { lastAnchor=anchor; }
	if(lastRotAngle!=rotAngle) { lastRotAngle=rotAngle; }
	if(changed) { lastModelMat=translate(Vector3(-anchor, 0.0f))*axisRotMat(rotAxis, deg_to_rad(rotAngle))*scaleMat(Vector3(scale, 1.0f))*translate(Vector3(position, zIndex-100.0f)); }
	return lastModelMat;
}
#pragma endregion// Transforms

#pragma region Shader
Shader::Shader(Engine* _engine, std::string vertexPath, std::string fragmentPath) : Object(_engine) {
	if(!initialized) return;
	// read vertex shader from file
	std::string vertexShaderSourceStr;
	FsReadDiskFile(&vertexShaderSourceStr, vertexPath);
	if(vertexShaderSourceStr.size()==0) {
		Log("File \""+vertexPath+"\" failed to read.");//error
		if(vertexPath=="Shaders/vs.glsl") vertexShaderSourceStr=vsShader;
		else if(vertexPath=="Shaders/textVs.glsl") vertexShaderSourceStr=textVsShader;
		else if(vertexPath=="Shaders/batchVs.glsl") vertexShaderSourceStr=batchVsShader;
		if(vertexShaderSourceStr.size()==0) {
			engine->Delete();
			return;
		} else {
			Log("Using hard coded shader for \""+vertexPath+"\".");
		}
	}
	const char* vertexShaderSource=vertexShaderSourceStr.c_str();
	unsigned int vertexShader;
	// create vertex shader
	vertexShader=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check if compiling vertex shader failed
	int vertexSuccess;
	//char vertexInfoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
	if(!vertexSuccess) {// glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
		glDeleteShader(vertexShader);
		Log("Vertex shader \""+vertexPath+"\" failed to init.");//error
		engine->Delete();
		return;

	}
	// read fragment shader from file
	std::string fragmentShaderSourceStr;
	FsReadDiskFile(&fragmentShaderSourceStr, fragmentPath);
	if(fragmentShaderSourceStr.size()==0) {
		Log("File \""+fragmentPath+"\" failed to read.");//error
		if(fragmentPath=="Shaders/colorFrag.glsl") fragmentShaderSourceStr=colorFragShader;
		else if(fragmentPath=="Shaders/texFrag.glsl") fragmentShaderSourceStr=texFragShader;
		else if(fragmentPath=="Shaders/textFrag.glsl") fragmentShaderSourceStr=textFragShader;
		else if(fragmentPath=="Shaders/batchFrag.glsl") fragmentShaderSourceStr=batchFragShader;
		if(fragmentShaderSourceStr.size()==0) {
			glDeleteShader(vertexShader);
			engine->Delete();
			return;
		} else {
			Log("Using hard coded shader for \""+fragmentPath+"\".");
		}
	}
	const char* fragmentShaderSource=fragmentShaderSourceStr.c_str();
	unsigned int fragmentShader;
	// create fragment shader
	fragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check if compiling fragment shader failed
	int fragmentSuccess;
	//char fragmentInfoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);
	if(!fragmentSuccess) {// glGetShaderInfoLog(fragmentSuccess, 512, NULL, fragmentInfoLog);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		Log(fragmentShaderSource);
		Log("Fragment shader \""+fragmentPath+"\" failed to init.");//error
		engine->Delete();
		return;

	}
	// create shader program
	program=glCreateProgram();
	// attach vertex and fragment shaders to program in order
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	int programSuccess;
	//char programInfoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &programSuccess);
	if(!programSuccess) {// glGetProgramInfoLog(shaderProgram, 512, NULL, programInfoLog);
		Log("Shader program failed to create.");//error
		Log("OpenGL version is");
		std::cout<<glGetString(GL_VERSION);
		engine->Delete();
		return;
	}
}
void Shader::use() {
	if(engine->ended||!initialized) return;
	glUseProgram(program);
}
Shader::~Shader() {
	if(!initialized) return;
	glDeleteProgram(program);
}
// uniform utility functions
void Shader::setBool(const char* name, const bool& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1i(uniformLocation, (bool)value);
}
void Shader::setInt(const char* name, const int& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1i(uniformLocation, value);
}
void Shader::setFloat(const char* name, const float& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1f(uniformLocation, value);
}
void Shader::setFloat2(const char* name, const Vector2& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform2f(uniformLocation, value.x, value.y);
}
void Shader::setFloat3(const char* name, const Vector3& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}
void Shader::setFloat4(const char* name, const Vector4& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
}
void Shader::setMat4x4(const char* name, const Mat4x4& value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value.values[0]);
}
void Shader::setTexture(const char* name, Texture* tex, const unsigned int& location) {
	if(engine->ended||!initialized||!tex->initialized) return;
	for(unsigned int i=0; i<textureIndexes.size(); i++) {
		if(textureIndexes[i]==location) { textures[i]=tex; return; }// replace texture at location if there already is one there
	}
	if(name!="_") setInt(name, location);
	textures.push_back(tex);
	textureIndexes.push_back(location);
}
void Shader::setTextureArray(const std::string& name) {
	if(engine->ended||!initialized) return;
	for(int i=0; i<maxTextures; i++) {
		setInt((name+"["+std::to_string(i)+"]").c_str(), i);
	}
}
void Shader::bindTextures() {
	if(engine->ended||!initialized) return;
	use();
	unsigned int len=std::min(textures.size(), textureIndexes.size());
	for(unsigned int i=0; i<len; i++) textures[i]->Bind(textureIndexes[i]);
}
#pragma endregion// Shader
#pragma region Texture
int load_texture(unsigned int* texture, std::string path, int* width, int* height) {
	int nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data=stbi_load(path.c_str(), width, height, &nrChannels, 0);//read raw image data from file
	if(data) {
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);// bind texture so that following code will assign the texture
		// texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// turn byte data into texture
		unsigned int channelsEnum=((nrChannels==3) ? GL_RGB : GL_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, channelsEnum, *width, *height, 0, channelsEnum, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);// free the memory holding the image data
		return 1;
	} else {
		return 0;
	}
}
Texture::Texture(Engine* _engine, std::string _path) :
	Object(_engine), ID(0), path(_path), width(0), height(0) {
	if(!initialized) return;
	if(!load_texture(&ID, path, &width, &height)) {
		initialized=false;
		Log("texture \""+path+"\" failed to load.");//error
		engine->Delete();
		return;
	}
}
void Texture::Bind(const unsigned int& location) {
	if(engine->ended||!initialized) return;
	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
#pragma endregion// Texture

#pragma region Cameras
Camera::Camera(Engine* _engine) : Object(_engine), projection(Mat4x4()), view(Mat4x4()) {
	if(!initialized) return;
}
void Camera::update() {}
void Camera::bindShader(Shader* shader) {
	if(engine->ended||!initialized||!shader->initialized) return;
	shaders.push_back(shader);
}
void Camera::bindShaders(std::vector<Shader*> shaders) {
	for(unsigned int i=0; i<shaders.size(); i++) bindShader(shaders[i]);
}
void Camera::use() {
	if(engine->ended||!initialized) return;
	for(unsigned int i=0; i<shaders.size(); i++) {
		Shader* ptr=(shaders)[i];
		if(!ptr->initialized) continue;
		ptr->setMat4x4("projection", projection);
		ptr->setMat4x4("view", view);
	}
}

LookAtCam::LookAtCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _focus) :
	Camera(_engine), Transform(_position, Vector3(), Vector3(), 0.0f), aspect(_aspect), focus(_focus) {
	if(!initialized) return;
	update();
}
void LookAtCam::update() {
	if(engine->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}

FreeCam::FreeCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _forward, Vector3 _up) :
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
void FreeCam::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
void FreeCam::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	if(engine->ended||!initialized) return;
	fov-=(float)yoffset;
	fov=clamp(fov, 1.0f, 90.0f);
	update();
}
void FreeCam::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {
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
void FreeCam::on_loop(double delta) {
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

OrthoCam::OrthoCam(Engine* _engine, Vector2 _position, Vector2 _scale) :
	Camera(_engine), Transform2D(_position, 0.0f, _scale, Vector2(), 0.0f) {
	if(!initialized) return;
	update();
}
void OrthoCam::update() {
	if(engine->ended||!initialized) return;
	projection=ortho(-scale.x/2.0f, scale.x/2.0f, -scale.y/2.0f, scale.y/2.0f, 0.0f, 1000.0f);
	view=translate(Vector3(-position, 0));
}
#pragma endregion// Cameras

#pragma region Renderer
Renderer::Renderer(Engine* _engine, Shader* _shader) : Object(_engine), shader(_shader), VAO(0), VBO(0), EBO(0) { if(!initialized||!shader->initialized) initialized=false; }
void Renderer::setShader(Shader* _shader) { shader=_shader; }
Renderer::~Renderer() {
	if(!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
#pragma endregion// Renderer
#pragma region CubeRenderer
float cubevertices[]={
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f,//0
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,//1
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,//2
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f,//3
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,//4
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,//5
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,//6
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,//7
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,//8
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,//9
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f,//10
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,//11
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,//12
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f,//13
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,//14
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f,//15
	0.5f, -0.5f, 0.5f, 1.0f, 1.0f,//16
	-0.5f, -0.5f, 0.5f, 0.0f, 1.0f,//17
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,//18
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,//19
};
int cubeindices[]={
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12,
	8, 13, 16, 16, 17, 8,
	18, 19, 14, 14, 11, 18
};
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _scale, Vector3 _rotAxis, float _rotAngle) :
	Renderer(_engine, _shader), Transform(_position, _scale, _rotAxis, _rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubevertices), cubevertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeindices), cubeindices, GL_STATIC_DRAW);// fill EBO buffer with index data
	// start, length, type, ?, total size, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
}
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _scale) : CubeRenderer(_engine, _shader, _position, _scale, Vector3::UP, 0.0f) {}
void CubeRenderer::draw() {
	if(engine->ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	shader->bindTextures();
	shader->setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
#pragma endregion// CubeRenderer
#pragma region Renderer2D
bool Renderer2D::AABBOverlap(const Vector2& aPos, const Vector2& aSize, const Vector2& bPos, const Vector2& bSize) {
	if(aPos==bPos) return true;// guaranteed collition
	// collision x-axis?
	float collisionX1=((aPos.x+aSize.x/2)-(bPos.x-bSize.x/2));
	float collisionX2=((bPos.x+bSize.x/2)-(aPos.x-aSize.x/2));
	// collision y-axis?
	float collisionY1=((aPos.y+aSize.y/2)-(bPos.y-bSize.y/2));
	float collisionY2=((bPos.y+bSize.y/2)-(aPos.y-aSize.y/2));
	// collision only if on both axes
	return collisionX1>0&&collisionX2>0&&collisionY1>0&&collisionY2>0;
}
bool Renderer2D::shouldDraw(const Vector2& viewer, const Vector2& viewRange) {
	return AABBOverlap(position-Vector2(anchor.x*scale.x, anchor.y*scale.y), scale, viewer, viewRange);
}
bool Renderer2D::shouldDraw(const Vector2& viewer, const float& viewRange) { return shouldDraw(viewer, Vector2(viewRange, viewRange)); }
bool Renderer2D::shouldDraw(OrthoCam* viewer) { return shouldDraw(viewer->position, viewer->scale); }
#pragma endregion// Renderers2D
#pragma region SpriteRenderer
float quadvertices[]={
	-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -1.0f, 0.0f, 1.0f,
	0.5f, 0.5f, -1.0f, 1.0f, 0.0f,
	0.5f, -0.5f, -1.0f, 1.0f, 1.0f
};
int quadindices[]={
	1, 3, 2, 2, 0, 1
};
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor, float _rotAngle) :
	Renderer2D(_engine, _shader, _position, _zIndex, _scale, _anchor, _rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadvertices), quadvertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale, Vector2 _anchor) : SpriteRenderer(_engine, _shader, _position, _zIndex, _scale, _anchor, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex, Vector2 _scale) : SpriteRenderer(_engine, _shader, _position, _zIndex, _scale, Vector2::Center, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _zIndex) : SpriteRenderer(_engine, _shader, _position, _zIndex, Vector2::ONE, Vector2::Center, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position) : SpriteRenderer(_engine, _shader, _position, 0.0f, Vector2::ONE, Vector2::Center, 0.0f) {}
void SpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", getModelMat());
	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
#pragma endregion// SpriteRenderer
#pragma region TextRenderer
struct Character {
	unsigned int TextureID=0;// ID handle of the glyph texture
	Vector2   Size;		     // Size of glyph
	Vector2   Bearing;       // Offset from baseline to left/top of glyph
	int Advance=0;  // Offset to advance to next glyph
};
std::map<char, Character> Characters;
bool characterMapInitialized=false;
int initCharacterMap() {
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) return 0;
	FT_Face face;
	if(FT_New_Face(ft, "Fonts/MonocraftBetterBrackets.ttf", 0, &face)) return 0;
	FT_Set_Pixel_Sizes(face, 0, 8);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
	for(unsigned char c=0; c<128; c++) {
		// load character glyph
		if(FT_Load_Char(face, c, FT_LOAD_RENDER)) return 0;
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// now store character for later use
		Vector2 Size((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows);
		Vector2 Bearing((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top);
		Character character={
			texture,
			Size,
			Bearing,
			((int)face->glyph->advance.x)>>6// bitshift by 6 to get value in pixels (2^6 = 64)
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	characterMapInitialized=true;
	return 1;
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale, float _zIndex, Vector2 _anchor) :
	Renderer2D(_engine, _shader, _position, _zIndex, Vector2::ONE, _anchor, 0.0f), text(_text), color(_color), scale(_scale) {
	if(!initialized) return;
	if(!characterMapInitialized) {
		if(!initCharacterMap()) {
			initialized=false;
			Log("Error initializing font \"Fonts/MonocraftBetterBrackets.ttf\"");//error
			engine->Delete();
			return;
		}
	}
	//setup buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadvertices), quadvertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader->setFloat("text", 0);
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale, float _zIndex) : TextRenderer(_engine, _shader, _text, _color, _position, _scale, _zIndex, Vector2::Center) {}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector3 _color, Vector2 _position, float _scale) : TextRenderer(_engine, _shader, _text, _color, _position, _scale, 0.0f, Vector2::Center) {}
void TextRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->setFloat3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	// iterate through all characters to find final scale
	int x=0;
	int maxX=0;
	int y=0;
	std::string::const_iterator c2;
	for(c2=text.begin(); c2!=text.end(); c2++) {
		if(*c2=='\n') {
			if(x>maxX)maxX=x;
			x=0;
			y-=9;
			continue;
		}
		Character ch=Characters[*c2];
		if(*c2==' ') { x+=1+ch.Advance; continue; }// skip one space and continue
		else if(*c2=='\t') { x+=1+ch.Advance*4; continue; }//4 character spaces
		x+=ch.Advance; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	if(x>maxX)maxX=x;
	Transform2D::scale=Vector2((float)maxX, (float)y)*scale;
	Mat4x4 casheMat=translate(Vector3(-anchor, 0.0f))*scaleMat(Vector3(scale, scale, 1.0f));
	Vector2 offset=position+Vector2((Transform2D::scale.x*(-anchor.x-0.5f)), (Transform2D::scale.y*(anchor.y-0.5f)));
	// iterate through all characters and render each
	x=0;
	y=0;
	std::string::const_iterator c;
	for(c=text.begin(); c!=text.end(); c++) {
		if(*c=='\n') { x=0;y-=9; continue; } else if(*c=='\r') { x=0; continue; }
		Character ch=Characters[*c];
		if(*c==' ') { x+=1+ch.Advance; continue; }// skip one space and continue
		else if(*c=='\t') { x+=1+ch.Advance*4; continue; }//4 character spaces
		Mat4x4 model=casheMat*scaleMat(Vector3(ch.Size, 1.0f))*translate(Vector3(offset+Vector2(((float)x)+ch.Bearing.x, ((float)y)-(ch.Size.y-ch.Bearing.y))*scale, zIndex-100.0f));
		shader->setMat4x4("model", model);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// render quad
		//glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		// now advance cursors for next glyph
		x+=1+ch.Advance;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma endregion// TextRenderer
#pragma region LineRenderer
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position, bool _loop) :
	Renderer2D(_engine, _shader, _position, 100.0f, Vector2::ONE, Vector2::Center, 0.0f), positions(_positions), width(_width), loop(_loop) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	float farthestX=0.0f;
	float farthestY=0.0f;
	std::vector<float> verts;
	for(Vector2 pos : positions) {
		Vector2 absPos=pos.abs();
		if(absPos.x>farthestX) farthestX=absPos.x;
		if(absPos.y>farthestY) farthestY=absPos.y;
		verts.push_back(pos.x);
		verts.push_back(pos.y);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
	}
	scale=Vector2(farthestX*2.0f, farthestY*2.0f);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position) : LineRenderer(_engine, _shader, _positions, _width, _position, false) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, bool _loop) : LineRenderer(_engine, _shader, _positions, _width, Vector2::ZERO, _loop) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width) : LineRenderer(_engine, _shader, _positions, _width, Vector2::ZERO, false) {}
void LineRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", createModelMat(position, zIndex));
	glBindVertexArray(VAO);
	glLineWidth(width);
	glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, positions.size());
}
#pragma endregion// LineRenderer
#pragma region BatchedSpriteRenderer
void BatchedSpriteRenderer::bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(numQuads>=maxQuadCount) renderBatch();// if out of room render and reset
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*dataBufferPtr=BatchedVertex {
			position.x+quadvertices[i+0]*scale.x, position.y+quadvertices[i+1]*scale.y,
			zIndex-100.0f,
			quadvertices[i+3], quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		dataBufferPtr++;
	}
	numQuads++;
}
void BatchedSpriteRenderer::renderBatch() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ((unsigned int)dataBufferPtr)-((unsigned int)dataBuffer), dataBuffer);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
	drawCalls++;
	dataBufferPtr=dataBuffer;
	numQuads=0;
}
BatchedSpriteRenderer::BatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), dataBuffer(nullptr), dataBufferPtr(nullptr) {
	if(!initialized) return;

	dataBuffer=new BatchedVertex[maxQuadCount*4];
	dataBufferPtr=dataBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=((unsigned int)i*4+1);
		indices[i*6+1]=((unsigned int)i*4+3);
		indices[i*6+2]=((unsigned int)i*4+2);
		indices[i*6+3]=((unsigned int)i*4+2);
		indices[i*6+4]=((unsigned int)i*4+0);
		indices[i*6+5]=((unsigned int)i*4+1);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*maxQuadCount*6, indices, GL_STATIC_DRAW);
	delete[] indices;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(9*sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
BatchedSpriteRenderer::~BatchedSpriteRenderer() { delete[] dataBuffer; }
void BatchedSpriteRenderer::addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	quads.push_back(QuadData { position, zIndex, scale, texIndex, modulate });
}
void BatchedSpriteRenderer::draw() {
	drawCalls=0;
	shader->bindTextures();
	for(const QuadData& quad:quads) bufferQuad(quad.position, quad.zIndex, quad.scale, quad.modulate, quad.texIndex);
	renderBatch();
}
#pragma endregion// BatchedSpriteRenderer
#pragma region StaticBatchedSpriteRenderer
void StaticBatchedSpriteRenderer::bufferQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	if(numQuads>=maxQuadCount) return;//dont try to add more than is possible, this should not occur though
	for(unsigned int i=0; i<((unsigned int)4*5); i+=5) {
		*dataBufferPtr=BatchedVertex {
			position.x+quadvertices[i+0]*scale.x, position.y+quadvertices[i+1]*scale.y,
			zIndex-100.0f,
			quadvertices[i+3], quadvertices[i+4],
			modulate.x, modulate.y, modulate.z, modulate.w,
			texIndex
		};
		dataBufferPtr++;
	}
	numQuads++;
}
void StaticBatchedSpriteRenderer::renderBatch() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numQuads*6, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, numQuads*4);
}
StaticBatchedSpriteRenderer::StaticBatchedSpriteRenderer(Engine* _engine, Shader* _shader) :
	Renderer2D(_engine, _shader, Vector2::ZERO, 0.0f, Vector2::ONE, Vector2::Center, 0.0f), dataBuffer(nullptr), dataBufferPtr(nullptr) {
	if(!initialized) return;

	dataBuffer=new BatchedVertex[maxQuadCount*4];
	dataBufferPtr=dataBuffer;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, maxQuadCount*4*sizeof(BatchedVertex), nullptr, GL_DYNAMIC_DRAW);
	unsigned int* indices=new unsigned int[maxQuadCount*6];
	for(int i=0; i<maxQuadCount; i++) {
		indices[i*6+0]=((unsigned int)i*4+1);
		indices[i*6+1]=((unsigned int)i*4+3);
		indices[i*6+2]=((unsigned int)i*4+2);
		indices[i*6+3]=((unsigned int)i*4+2);
		indices[i*6+4]=((unsigned int)i*4+0);
		indices[i*6+5]=((unsigned int)i*4+1);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*maxQuadCount*6, indices, GL_STATIC_DRAW);
	delete[] indices;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchedVertex), (void*)(9*sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
StaticBatchedSpriteRenderer::~StaticBatchedSpriteRenderer() { delete[] dataBuffer; }
void StaticBatchedSpriteRenderer::addQuad(const Vector2& position, const float& zIndex, const Vector2& scale, const Vector4& modulate, const float& texIndex) {
	quads.push_back(QuadData { position, zIndex, scale, texIndex, modulate });
}
void StaticBatchedSpriteRenderer::bind() {
	dataBufferPtr=dataBuffer;
	numQuads=0;
	for(const QuadData& quad:quads) bufferQuad(quad.position, quad.zIndex, quad.scale, quad.modulate, quad.texIndex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ((unsigned int)dataBufferPtr)-((unsigned int)dataBuffer), dataBuffer);
}
void StaticBatchedSpriteRenderer::draw() {
	shader->bindTextures();
	renderBatch();
}
#pragma endregion// StaticBatchedSpriteRenderer

#pragma region Stencil
void StencilSimple::Enable() {
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}
void StencilSimple::Write() {
	glStencilFunc(GL_ALWAYS, 1, 0b0);// compare against none
	glStencilMask(0b1);// write to 0b1
}
void StencilSimple::Compare() {
	glStencilFunc(GL_EQUAL, 1, 0b1);// compare against 0b1
	glStencilMask(0b0);// write to none
}
void StencilSimple::Disable() {
	glDisable(GL_STENCIL_TEST);
}
#pragma endregion// Stencil
