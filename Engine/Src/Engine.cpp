#include "Engine.h"

#define STB_IMAGE_IMPLEMENTATION// not entirely sure what this is for
#include <stb_image.h>

#pragma region Engine
void engine_on_error(int error, const char* description) {
	Log("GLDW error: "+std::string(description));
#ifdef _DEBUG
	__debugbreak();
#endif
}

#pragma region callbacks
void Engine::on_resize(GLFWwindow* window, int width, int height) {
	if(ended||!initialized) return;
	glViewport(0, 0, width, height); screenSize=Vector2((float)width, (float)height);
	for(Object* obj:onResize) {
		obj->on_resize(window, width, height);
	}
}
void Engine::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(ended||!initialized) return;
	for(Object* obj:onKey) {
		obj->on_key(window, key, scancode, action, mods);
	}
}
void Engine::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	if(ended||!initialized) return;
	for(Object* obj:onScroll) {
		obj->on_scroll(window, xoffset, yoffset);
	}
}
void Engine::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {
	if(ended||!initialized) return;
	if(lastMouse.x==-1||lastMouse.y==-1) {
		lastMouse.x=(float)mouseX; lastMouse.y=(float)mouseY; return;
	}
	float deltaX=((float)mouseX)-lastMouse.x;
	float deltaY=lastMouse.y-((float)mouseY);
	lastMouse=Vector2((float)mouseX, (float)mouseY);
	for(Object* obj:onMouse) {
		obj->on_mouse(window, mouseX, mouseY);
	}
	if((deltaX!=0)||(deltaY!=0)) on_mouse_delta(window, deltaX, deltaY);
}
void Engine::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseDelta) {
		obj->on_mouse_delta(window, deltaX, deltaY);
	}
}
void Engine::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseButton) {
		obj->on_mouse_button(window, button, action, mods);
	}
}
void Engine::on_mouse_enter(GLFWwindow* window, int entered) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseEnter) {
		obj->on_mouse_enter(window, entered);
	}
}
#pragma endregion// callbacks

Engine::Engine(const Vector2& size, const char* title, const bool& vsync) : window(nullptr), screenSize(size) {
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
void Engine::SetCursorMode(const int& mode) {
	if(ended||!initialized) return;
	glfwSetInputMode(window, GLFW_CURSOR, mode);
	lastMouse=Vector2(-1.0f, -1.0f);
}

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
template <typename T> void vecRemoveValue(std::vector<T>& vec, const T& value) {
	for(unsigned int i=0; i<vec.size(); i++) {
		if(vec[i]==value) vec.erase(vec.begin()+i);
	}
}

void Object::on_resize(GLFWwindow* window, const int& width, const int& height) {}
void Object::on_key(GLFWwindow* window, const int& key, const int& scancode, const int& action, const int& mods) {}
void Object::on_scroll(GLFWwindow* window, const double& xoffset, const double& yoffset) {}
void Object::on_mouse(GLFWwindow* window, const double& mouseX, const double& mouseY) {}
void Object::on_mouse_delta(GLFWwindow* window, const float& deltaX, const float& deltaY) {}
void Object::on_mouse_button(GLFWwindow* window, const int& button, const int& action, const int& mods) {}
void Object::on_mouse_enter(GLFWwindow* window, const int& entered) {}
void Object::on_loop(const double& delta) {}
Object::Object(Engine* _engine) : engine(_engine) {
	if(!engine->initialized||engine->ended) { initialized=false; return; }
	initialized=true;
	engine->objects.push_back(this);
}
Object::~Object() {
	if(!initialized) return;
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
#pragma endregion// Object

#pragma region Transforms
Transform::Transform(const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle) :
	position(_position), scale(_scale), rotAxis(_rotAxis), rotAngle(_rotAngle) {}
// Transform2D
Transform2D::Transform2D(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	position(_position), zIndex(_zIndex), scale(_scale), anchor(_anchor), rotAngle(_rotAngle) {
	lastModelMat=translate(Vector3(-anchor, 0.0f))*axisRotMat(rotAxis, deg_to_rad(rotAngle))*scaleMat(Vector3(scale, 1.0f))*translate(Vector3(position, zIndex-100.0f));
}
Vector2 Transform2D::getWorldPos() const {
	if(parent!=nullptr) return parent->getWorldPos()+position;
	else return position;
}
Vector2 Transform2D::getWorldScale() const {
	if(parent!=nullptr) {
		Vector2 parentWorldScale=parent->getWorldScale();
		return Vector2(parentWorldScale.x*scale.x, parentWorldScale.y*scale.y);
	} else return scale;
}
float Transform2D::getWorldRot() const {
	if(parent!=nullptr) return parent->getWorldRot()+rotAngle;
	else return rotAngle;
}
void Transform2D::addChild(Transform2D* child) {
	for(Transform2D* c:children) if(c==child) return;
	if(child->parent!=nullptr) {//remove child from its parents list of children
		child->parent->children.erase(std::find(child->parent->children.begin(), child->parent->children.end(), child));
	}
	child->parent=this;
	children.push_back(child);
}
Mat4x4 Transform2D::createModelMat(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) {
	return translate(Vector3(-_anchor, 0.0f))*axisRotMat(Vector3(0.0f, 0.0f, 1.0f), deg_to_rad(_rotAngle))*scaleMat(Vector3(_scale, 1.0f))*translate(Vector3(_position, _zIndex-100.0f));
}
Mat4x4 Transform2D::getModelMat() {
	bool changed=false;
	Vector2 worldPos=getWorldPos();
	Vector2 worldScale=getWorldScale();
	float worldRot=getWorldRot();
	if(lastWorldPosition!=worldPos) { lastWorldPosition=worldPos;changed=true; }
	if(lastZIndex!=zIndex) { lastZIndex=zIndex;changed=true; }
	if(lastWorldScale!=worldScale) { lastWorldScale=worldScale;changed=true; }
	if(lastWorldRot!=worldRot) { lastWorldRot=worldRot;changed=true; }
	if(changed) { lastModelMat=translate(Vector3(-anchor, 0.0f))*axisRotMat(rotAxis, deg_to_rad(worldRot))*scaleMat(Vector3(worldScale, 1.0f))*translate(Vector3(worldPos, zIndex-100.0f)); }
	return lastModelMat;
}
#pragma endregion// Transforms

#pragma region Shader
Shader::Shader(Engine* _engine, const std::string& vertexPath, const std::string& fragmentPath) : Object(_engine) {
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
Shader::~Shader() {
	if(!initialized) return;
	glDeleteProgram(program);
}
void Shader::use() {
	if(engine->ended||!initialized) return;
	glUseProgram(program);
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
	numTextures++;
}
void Shader::setTextureArray(const std::string& name) {
	if(engine->ended||!initialized) return;
	for(int i=0; i<32; i++) {// max slots on the gpu for textures
		setInt((name+"["+std::to_string(i)+"]").c_str(), i);
	}
}
void Shader::bindTextures() {
	if(engine->ended||!initialized) return;
	use();
	for(unsigned int i=0; i<numTextures; i++) textures[i]->Bind(textureIndexes[i]);
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

void Texture::Bind(const unsigned int& location) {
	if(engine->ended||!initialized) return;
	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
Texture::Texture(Engine* _engine, const std::string& _path) :
	Object(_engine), ID(0), path(_path), width(0), height(0) {
	if(!initialized) return;
	if(!load_texture(&ID, path, &width, &height)) {
		initialized=false;
		Log("texture \""+path+"\" failed to load.");//error
		engine->Delete();
		return;
	}
}
#pragma endregion// Texture

#pragma region StencilSimple
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
#pragma endregion// StencilSimple