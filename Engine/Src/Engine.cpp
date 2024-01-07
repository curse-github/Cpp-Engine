#include "Engine.h"

#define STB_IMAGE_IMPLEMENTATION// not entirely sure what this is for
#include <stb_image.h>

#pragma region Engine
void engine_on_error(int error, const char* description) {
	Log("GLFW error: "+std::string(description));
#ifdef _DEBUG
	__debugbreak();
#endif
}

#pragma region callbacks
void Engine::on_resize(int width, int height) {
	if(ended||!initialized) return;
	glViewport(0, 0, width, height);
	curResolution=Vector2(static_cast<float>(width), static_cast<float>(height));
	for(Object* obj:onResize) {
		obj->on_resize(width, height);
	}
}
void Engine::on_key(int key, int scancode, int action, int mods) {
	if(ended||!initialized) return;
	for(Object* obj:onKey) {
		obj->on_key(key, scancode, action, mods);
	}
}
void Engine::on_scroll(double xoffset, double yoffset) {
	if(ended||!initialized) return;
	for(Object* obj:onScroll) {
		obj->on_scroll(xoffset, yoffset);
	}
}
void Engine::on_mouse(double mouseX, double mouseY) {
	if(ended||!initialized) return;
	if(curMousePos.x==-1||curMousePos.y==-1) {
		curMousePos.x=static_cast<float>(mouseX); curMousePos.y=static_cast<float>(mouseY); return;
	}
	float deltaX=static_cast<float>(mouseX)-curMousePos.x;
	float deltaY=curMousePos.y-static_cast<float>(mouseY);
	curMousePos=Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
	for(Object* obj:onMouse) {
		obj->on_mouse(mouseX, mouseY);
	}
	if((deltaX!=0)||(deltaY!=0)) on_mouse_delta(deltaX, deltaY);
}
void Engine::on_mouse_delta(float deltaX, float deltaY) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseDelta) {
		obj->on_mouse_delta(deltaX, deltaY);
	}
}
void Engine::on_mouse_button(int button, int action, int mods) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseButton) {
		obj->on_mouse_button(button, action, mods);
	}
}
void Engine::on_mouse_enter(int entered) {
	if(ended||!initialized) return;
	for(Object* obj:onMouseEnter) {
		obj->on_mouse_enter(entered);
	}
}
#pragma endregion// callbacks

Engine::Engine(const Vector2& size, const char* title, const bool& vsync) : window(nullptr), curResolution(size) {
	instance=this;
	if(glfwInit()==GLFW_FALSE) {
		ended=true;
		Log("GLFW failed to init.");//error
		return;
	}

	window=glfwCreateWindow(static_cast<int>(size.x), static_cast<int>(size.y), title, NULL, NULL);
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
	glViewport(0, 0, static_cast<int>(size.x), static_cast<int>(size.y));
	glfwSwapInterval(static_cast<int>(vsync));// V-Sync: 1=on, 0=off

	glfwSetErrorCallback(engine_on_error);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {Engine::instance->on_resize(width, height); });
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {Engine::instance->on_key(key, scancode, action, mods); });
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {Engine::instance->on_scroll(xoffset, yoffset); });
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double mouseX, double mouseY) {Engine::instance->on_mouse(mouseX, mouseY); });
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {Engine::instance->on_mouse_button(button, action, mods); });
	glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) {Engine::instance->on_mouse_enter(entered); });

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	// makes caps lock detectable
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

	initialized=true;
}
Engine::~Engine() {
	if(!initialized) return;
	while(objects.size()>0) {
		Object* ptr=objects[0];
		if(ptr) delete ptr;
	}
}
void Engine::Start() {
	if(ended||!initialized) return;
	for(Object* obj:onStart) {
		obj->on_start();
	}
	Loop();
}
void Engine::Loop() {
	if(ended||!initialized) return;
	double lastFrameTime=glfwGetTime();
	while(!glfwWindowShouldClose(window)) {
		double delta=glfwGetTime()-lastFrameTime;
		lastFrameTime=glfwGetTime();
#pragma region calc fps
		for(unsigned int i=1; i<FpsAvgNum; i++) { fpsData.deltaHistory[i-1]=fpsData.deltaHistory[i]; }// move values back
		fpsData.deltaHistory[FpsAvgNum-1]=delta;// put delta at the end
		double sum=0.0f;
		for(unsigned int i=0; i<FpsAvgNum; i++) { sum+=fpsData.deltaHistory[i]; }// sum values
		fpsAvg=static_cast<int>(FpsAvgNum/sum+0.5);
		frameTimeAvg=static_cast<float>(sum);//1000.0f*1000.0f;

		fpsHigh=0;
		fpsLow=1000000;
		for(unsigned int i=0; i<FpsAvgNum; i++) {
			double fps=1/fpsData.deltaHistory[i];
			if(fps>fpsHigh) fpsHigh=static_cast<unsigned int>(fps+0.5);
			if(fps<fpsLow) fpsLow=static_cast<unsigned int>(fps+0.5);
		}
#pragma endregion// calc fps

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		drawCalls=curDrawCalls;
		curDrawCalls=0;
		for(Object* obj:onLoop) {
			obj->on_loop(delta);
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
	curMousePos=Vector2(-1.0f, -1.0f);
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
void Engine::sub_start(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onStart.push_back(obj);
}
void Engine::sub_loop(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onLoop.push_back(obj);
}
#pragma endregion// subFuncs

Engine* Engine::instance=nullptr;
#pragma endregion// Engine
#pragma region Object
template <typename T> void vecRemoveValue(std::vector<T>& vec, const T& value) {
	for(unsigned int i=0; i<vec.size(); i++) {
		if(vec[i]==value) vec.erase(vec.begin()+i);
	}
}

void Object::on_resize(const int& width, const int& height) {}
void Object::on_key(const int& key, const int& scancode, const int& action, const int& mods) {}
void Object::on_scroll(const double& xoffset, const double& yoffset) {}
void Object::on_mouse(const double& mouseX, const double& mouseY) {}
void Object::on_mouse_delta(const float& deltaX, const float& deltaY) {}
void Object::on_mouse_button(const int& button, const int& action, const int& mods) {}
void Object::on_mouse_enter(const int& entered) {}
void Object::on_start() {}
void Object::on_loop(const double& delta) {}
Object::Object() {
	if(Engine::instance==nullptr||!Engine::instance->initialized||Engine::instance->ended) { initialized=false; return; }
	initialized=true;
	Engine::instance->objects.push_back(this);
}
Object::~Object() {
	if(!initialized) return;
	vecRemoveValue(Engine::instance->objects, this);
	vecRemoveValue(Engine::instance->onResize, this);
	vecRemoveValue(Engine::instance->onKey, this);
	vecRemoveValue(Engine::instance->onScroll, this);
	vecRemoveValue(Engine::instance->onMouse, this);
	vecRemoveValue(Engine::instance->onMouseDelta, this);
	vecRemoveValue(Engine::instance->onMouseButton, this);
	vecRemoveValue(Engine::instance->onMouseEnter, this);
	vecRemoveValue(Engine::instance->onLoop, this);
}
#pragma endregion// Object

#pragma region Transforms
Transform::Transform(const Vector3& _position, const Vector3& _scale, const Vector3& _rotAxis, const float& _rotAngle) :
	position(_position), scale(_scale), rotAxis(_rotAxis), rotAngle(_rotAngle) {}
// Transform2D
bool Transform2D::AABBOverlap(const Vector2& aPos, const Vector2& aSize, const Vector2& bPos, const Vector2& bSize) {
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

Transform2D::Transform2D(const Vector2& _position, const float& _zIndex, const Vector2& _scale, const Vector2& _anchor, const float& _rotAngle) :
	active(true), position(_position), zIndex(_zIndex), scale(_scale), anchor(_anchor), rotAngle(_rotAngle) {
	lastModelMat=translate(Vector3(-anchor, 0.0f))*axisRotMat(rotAxis, deg_to_rad(rotAngle))*scaleMat(Vector3(scale, 1.0f))*translate(Vector3(position, zIndex-100.0f));
}
Transform2D::~Transform2D() {
	if(parent!=nullptr) {//remove child from its parents list of children
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
	}
	for(Transform2D* child:children) child->parent=nullptr;
}

bool Transform2D::inRange(const Vector2& viewer, const Vector2& viewRange) {
	if(Engine::instance->ended) return false;
	return isActive()&&AABBOverlap(getWorldPos()-Vector2(anchor.x*scale.x, anchor.y*scale.y), scale, viewer, viewRange);
}
bool Transform2D::inRange(const Vector2& viewer, const float& viewRange) {
	return inRange(viewer, Vector2(viewRange));
}

bool Transform2D::isActive() const {
	if(parent!=nullptr) return parent->isActive()&&active;
	else return active;
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

void Transform2D::setWorldPos(Vector2 pos) {
	if(parent!=nullptr) pos-parent->getWorldPos();
	else position=pos;
}
#pragma endregion// Transforms

#pragma region Shader
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) : Object() {
	if(!initialized) return;
	// read vertex shader from file
	std::string vertexShaderSourceStr;
	FsReadDiskFile(&vertexShaderSourceStr, vertexPath);
	if(vertexShaderSourceStr.size()==0) {
		Log("File \""+vertexPath+"\" failed to read.");//error
		if(vertexPath=="Shaders/basic.vert") vertexShaderSourceStr=vsShader;
		else if(vertexPath=="Shaders/batch.vert") vertexShaderSourceStr=batchVsShader;
		if(vertexShaderSourceStr.size()==0) {
			Engine::instance->Delete();
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
		Engine::instance->Delete();
		return;

	}
	// read fragment shader from file
	std::string fragmentShaderSourceStr;
	FsReadDiskFile(&fragmentShaderSourceStr, fragmentPath);
	if(fragmentShaderSourceStr.size()==0) {
		Log("File \""+fragmentPath+"\" failed to read.");//error
		if(fragmentPath=="Shaders/color.frag") fragmentShaderSourceStr=colorFragShader;
		else if(fragmentPath=="Shaders/tex.frag") fragmentShaderSourceStr=texFragShader;
		else if(fragmentPath=="Shaders/text.frag") fragmentShaderSourceStr=textFragShader;
		else if(fragmentPath=="Shaders/dotColor.frag") fragmentShaderSourceStr=dotColorFragShader;
		else if(fragmentPath=="Shaders/dotTex.frag") fragmentShaderSourceStr=dotTexFragShader;
		else if(fragmentPath=="Shaders/texBatch.frag") fragmentShaderSourceStr=texBatchFragShader;
		else if(fragmentPath=="Shaders/textBatch.frag") fragmentShaderSourceStr=textBatchFragShader;
		if(fragmentShaderSourceStr.size()==0) {
			glDeleteShader(vertexShader);
			Engine::instance->Delete();
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
		Engine::instance->Delete();
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
		Engine::instance->Delete();
		return;
	}
}
Shader::~Shader() {
	if(!initialized) return;
	glDeleteProgram(program);
}
void Shader::use() {
	if(Engine::instance->ended||!initialized) return;
	glUseProgram(program);
}
// uniform utility functions
void Shader::setBool(const char* name, const bool& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1i(uniformLocation, (bool)value);
}
void Shader::setInt(const char* name, const int& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1i(uniformLocation, value);
}
void Shader::setFloat(const char* name, const float& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform1f(uniformLocation, value);
}
void Shader::setFloat2(const char* name, const Vector2& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform2f(uniformLocation, value.x, value.y);
}
void Shader::setFloat3(const char* name, const Vector3& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}
void Shader::setFloat4(const char* name, const Vector4& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
}
void Shader::setMat4x4(const char* name, const Mat4x4& value) {
	if(Engine::instance->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name);
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value.values[0]);
}
void Shader::setTexture(const char* name, Texture* tex, const unsigned int& location) {
	if(Engine::instance->ended||!initialized||!tex->initialized) return;
	for(unsigned int i=0; i<textureIndexes.size(); i++) {
		if(textureIndexes[i]==location) { textures[i]=tex; return; }// replace texture at location if there already is one there
	}
	if(name!="_") setInt(name, location);
	textures.push_back(tex);
	textureIndexes.push_back(location);
	numTextures++;
}
void Shader::setTextureArray(const std::string& name) {
	if(Engine::instance->ended||!initialized) return;
	for(int i=0; i<32; i++) {// max slots on the gpu for textures
		setInt((name+"["+std::to_string(i)+"]").c_str(), i);
	}
}
void Shader::bindTexture(const unsigned int& index) {
	if(Engine::instance->ended||!initialized) return;
	use();
	if(index>=numTextures) return;
	// equivilent to '''textures[index]->Bind(textureIndexes[index]);'''
	glActiveTexture(GL_TEXTURE0+textureIndexes[index]);
	glBindTexture(GL_TEXTURE_2D, textures[index]->ID);
}
void Shader::bindTextures() {
	if(Engine::instance->ended||!initialized) return;
	use();
	for(unsigned int i=0; i<numTextures; i++) {
		// equivilent to '''textures[i]->Bind(textureIndexes[i]);'''
		glActiveTexture(GL_TEXTURE0+textureIndexes[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
	}
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

Texture::Texture(const unsigned int& _ID) :
	Object(), ID(_ID), width(0), height(0) {
	if(!initialized) return;
}
Texture::Texture(const std::string& path) :
	Object(), ID(0), width(0), height(0) {
	if(!initialized) return;
	if(!load_texture(&ID, path, &width, &height)) {
		initialized=false;
		Log("texture \""+path+"\" failed to load.");//error
		Engine::instance->Delete();
		return;
	}
}
void Texture::Bind(const unsigned int& location) {
	if(Engine::instance->ended||!initialized) return;
	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
#pragma endregion// Texture

#pragma region VertexArrayObject
VertexArrayObject::VertexArrayObject(const bool& thing) : VAO(0) {
	glGenVertexArrays(1, &VAO);
};
VertexArrayObject::~VertexArrayObject() {
	glDeleteVertexArrays(1, &VAO);
}
void VertexArrayObject::drawTris(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::drawTrisIndexed(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::drawTriStrip(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::drawTriStripIndexed(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::DrawLines(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawElements(GL_LINE, count, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::DrawLine(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, count);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
void VertexArrayObject::DrawLineLoop(const unsigned int& count) {
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_LOOP, 0, count);
	glBindVertexArray(0);
	Engine::instance->curDrawCalls++;
}
#pragma endregion// VertexArrayObject
#pragma region VertexBufferObject
VertexBufferObject::VertexBufferObject(VertexArrayObject* _VAO) : VBO(0), VAO(_VAO) {
	glGenBuffers(1, &VBO);
};
VertexBufferObject::~VertexBufferObject() {
	glDeleteBuffers(1, &VBO);
}
void VertexBufferObject::staticFill(const float* vertices, const size_t& len) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*len, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBufferObject::dynamicDefine(const size_t& len) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*len, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBufferObject::dynamicSub(const float* vertices, const size_t& len) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*len, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBufferObject::dynamicSub(const void* offset, const float* vertices, const size_t& len) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)offset, sizeof(float)*len, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBufferObject::applyAttributes(std::vector<unsigned int> attributes) {
	glBindVertexArray(VAO->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	unsigned int total=0;
	for(const unsigned int& attr:attributes)total+=attr;
	unsigned int count=0;
	for(unsigned int i=0;i<attributes.size();i++) {
		//index, size of location, type(GL_FLOAT), normalized(GL_FALSE), stride between vertexes, offset of current attribute
		glVertexAttribPointer(i, attributes[i], GL_FLOAT, GL_FALSE, sizeof(float)*total, (void*)(count*sizeof(float)));
		glEnableVertexAttribArray(i);// bind data above to (location = 1) in vertex shader
		count+=attributes[i];
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
#pragma endregion// VertexBufferObject
#pragma region IndexBufferObject
IndexBufferObject::IndexBufferObject(VertexArrayObject* _VAO) : EBO(0), VAO(_VAO) {
	glGenBuffers(1, &EBO);
};
IndexBufferObject::~IndexBufferObject() {
	glDeleteBuffers(1, &EBO);
}
void IndexBufferObject::fill(const unsigned int* indices, const size_t& len) {
	glBindVertexArray(VAO->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*len, indices, GL_STATIC_DRAW);// fill EBO buffer with index data
	glBindVertexArray(0);
}
void IndexBufferObject::fillRepeated(const unsigned int* indices, const size_t& len, const size_t& count, const unsigned int& numVertices) {
	glBindVertexArray(VAO->VAO);
	// Populates index buffer without allocating memory for thousands of ints
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*len*count, nullptr, GL_STATIC_DRAW);
	unsigned int* indicesData=static_cast<unsigned int*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));// Maps the buffer to update the data directly
	for(unsigned int i=0; i<static_cast<unsigned int>(count); i++) {// Populate the indices directly in the mapped buffer
		for(unsigned int j=0; j<static_cast<unsigned int>(len); j++) {
			indicesData[i*len+j]=i*numVertices+SpriteRenderer::quadindices[j];
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindVertexArray(0);
}
#pragma endregion// ElementBufferObject

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