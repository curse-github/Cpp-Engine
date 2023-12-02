#include "Engine.h"
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include <freetype/freetype.h>
#include <map>
#include <limits>
#define clamp(a,b,c) std::max(b,std::min(a,c))

#pragma region Engine
Engine::Engine(Vector2 size, const char* title, bool vsync) : window(nullptr), screenSize(size) {
	if(!glfwInit()) {
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
void Engine::Loop() {
	if(ended||!initialized) return;
	double lastFrameTime=glfwGetTime();
	while(!glfwWindowShouldClose(window)) {
		double delta=glfwGetTime()-lastFrameTime;
		lastFrameTime=glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		for(unsigned int i=0; i<onLoop.size(); i++) {
			onLoop[i](delta);
		}
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
	for_each(onDelete.begin(), onDelete.end(), [](ondeletefun callback) {
		callback();
		});
}

void Engine::SetCursor(int mode) {
	if(ended||!initialized) return;
	glfwSetInputMode(window, GLFW_CURSOR, mode);
	lastMouse=Vector2(-1.0f, -1.0f);
}

#pragma region callbacks
void engine_on_error(int error, const char* description) {
	Log("GLDW error: "+std::string(description));
#ifdef _DEBUG
	__debugbreak();
#endif
}
void Engine::on_resize(GLFWwindow* window, int width, int height) {
	if(ended||!initialized) return;
	glViewport(0, 0, width, height); screenSize=Vector2((float)width, (float)height);
	for_each(onResize.begin(), onResize.end(), [window, width, height](onresizefun callback) {
		callback(window, width, height);
		});
}
void Engine::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(ended||!initialized) return;
	for_each(onKey.begin(), onKey.end(), [window, key, scancode, action, mods](onkeyfun callback) {
		callback(window, key, scancode, action, mods);
		});
}
void Engine::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	if(ended||!initialized) return;
	for_each(onScroll.begin(), onScroll.end(), [window, xoffset, yoffset](onscrollfun callback) {
		callback(window, xoffset, yoffset);
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
	for_each(onMouse.begin(), onMouse.end(), [window, mouseX, mouseY](onmousefun callback) {
		callback(window, mouseX, mouseY);
		});
	if((deltaX!=0)||(deltaY!=0)) on_mouse_delta(window, deltaX, deltaY);
}
void Engine::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {
	if(ended||!initialized) return;
	for_each(onMouseDelta.begin(), onMouseDelta.end(), [window, deltaX, deltaY](onmousedeltafun callback) {
		callback(window, deltaX, deltaY);
		});
}
void Engine::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	if(ended||!initialized) return;
	for_each(onMouseButton.begin(), onMouseButton.end(), [window, button, action, mods](onmousebuttonfun callback) {
		callback(window, button, action, mods);
		});
}
void Engine::on_mouse_enter(GLFWwindow* window, int entered) {
	if(ended||!initialized) return;
	for_each(onMouseEnter.begin(), onMouseEnter.end(), [window, entered](onmouseenterfun callback) {
		callback(window, entered);
		});
}
#pragma endregion// callbacks

#pragma region subFuncs
void Engine::sub_resize(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onResize.push_back([=](GLFWwindow* window, int width, int height) {
		obj->on_resize(window, width, height);
		});
}
void Engine::sub_key(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onKey.push_back([=](GLFWwindow* window, int key, int scancode, int action, int mods) {
		obj->on_key(window, key, scancode, action, mods);
		});
}
void Engine::sub_scroll(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onScroll.push_back([=](GLFWwindow* window, double xoffset, double yoffset) {
		obj->on_scroll(window, xoffset, yoffset);
		});
}
void Engine::sub_mouse(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouse.push_back([=](GLFWwindow* window, double mouseX, double mouseY) {
		obj->on_mouse(window, mouseX, mouseY);
		});
}
void Engine::sub_mouse_delta(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseDelta.push_back([=](GLFWwindow* window, float deltaX, float deltaY) {
		obj->on_mouse_delta(window, deltaX, deltaY);
		});
}
void Engine::sub_mouse_button(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseButton.push_back([=](GLFWwindow* window, int button, int action, int mods) {
		obj->on_mouse_button(window, button, action, mods);
		});
}
void Engine::sub_mouse_enter(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onMouseEnter.push_back([=](GLFWwindow* window, int entered) {
		obj->on_mouse_enter(window, entered);
		});
}
void Engine::sub_delete(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onDelete.push_back([=]() {
		obj->on_delete();
		});
}
void Engine::sub_loop(Object* obj) {
	if(!initialized||ended||!obj->initialized) return;
	onLoop.push_back([=](double delta) {
		obj->on_loop(delta);
		});
}
#pragma endregion// subFuncs
#pragma endregion// Engine

#pragma region Object
Object::Object(Engine* _engine) : engine(_engine) {
	if(!engine->initialized||engine->ended) { initialized=false; return; }
	initialized=true;
}

void Object::on_resize(GLFWwindow* window, int width, int height) {}
void Object::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {}
void Object::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {}
void Object::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {}
void Object::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {}
void Object::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {}
void Object::on_mouse_enter(GLFWwindow* window, int entered) {}
void Object::on_delete() {}
void Object::on_loop(double delta) {}
#pragma endregion// Object

#pragma region Shader
Shader::Shader(Engine* _engine, std::string vertexPath, std::string fragmentPath) : Object(_engine) {
	if(!initialized) return;
	// read vertex shader from file
	std::string vertexShaderSourceStr;
	FsReadDiskFile(&vertexShaderSourceStr, vertexPath);
	if(vertexShaderSourceStr.size()==0) {
		Log("File \""+vertexPath+"\" failed to read.");//error
		if(vertexPath=="Shaders/vs.glsl") vertexShaderSourceStr=vsShader;
		if(vertexPath=="Shaders/textVs.glsl") vertexShaderSourceStr=textVsShader;
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
		if(fragmentPath=="Shaders/texFrag.glsl") fragmentShaderSourceStr=texFragShader;
		if(fragmentPath=="Shaders/textFrag.glsl") fragmentShaderSourceStr=textFragShader;
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
		on_delete();
		Log("Shader program failed to create.");//error
		engine->Delete();
		return;
	}
	engine->sub_delete(this);
}
void Shader::use() {
	if(engine->ended||!initialized) return;
	glUseProgram(program);
}
void Shader::on_delete() {
	if(!initialized) return;
	glDeleteProgram(program);
}
// uniform utility functions
void Shader::setBool(const std::string& name, bool value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, (bool)value);
}
void Shader::setInt(const std::string& name, int value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, value);
}
void Shader::setFloat(const std::string& name, float value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1f(uniformLocation, value);
}
void Shader::setFloat2(const std::string& name, Vector2 value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform2f(uniformLocation, value.x, value.y);
}
void Shader::setFloat3(const std::string& name, Vector3 value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}
void Shader::setFloat4(const std::string& name, Vector4 value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
}
void Shader::setMat4x4(const std::string& name, Mat4x4 value) {
	if(engine->ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value.values[0]);
}
void Shader::setTexture(const std::string& name, Texture* tex, unsigned int location) {
	if(engine->ended||!initialized||!tex->initialized) return;
	setInt(name, location);
	textures.push_back(tex);
	textureIndexes.push_back(location);
}
void Shader::bindTextures() {
	if(engine->ended||!initialized) return;
	for(unsigned int i=0; i<std::min(textures.size(), textureIndexes.size()); i++) {
		textures[i]->Bind(this, textureIndexes[i]);
	}
}
#pragma endregion// Shader

#pragma region Camera
// save reference to object as it is in the contructor to be used in the set function
// similar effect as glfwSetWindowUserPointer in the main function
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
	Camera(_engine), aspect(_aspect), position(_position), focus(_focus) {
	if(!initialized) return;
	update();
}
void LookAtCam::update() {
	if(engine->ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}

FreeCam::FreeCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _forward, Vector3 _up) :
	Camera(_engine), aspect(_aspect), position(_position), forward(_forward), up(_up) {
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
		engine->SetCursor(paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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

OrthoCam::OrthoCam(Engine* _engine, Vector2 _position, Vector2 _size) :
	Camera(_engine), position(_position), size(_size) {
	if(!initialized) return;
	update();
}
void OrthoCam::update() {
	if(engine->ended||!initialized) return;
	projection=ortho(-size.x/2.0f, size.x/2.0f, -size.y/2.0f, size.y/2.0f, 0.0f, 1000.0f);
	view=translate(Vector3(-position, 0));
}
#pragma endregion// Camera

#pragma region Texture
int load_texture(unsigned int* texture, std::string path, int* width, int* height) {
	int nrChannels;
	stbi_set_flip_vertically_on_load(true);
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
void Texture::Bind(Shader* shader, unsigned int location) {
	if(engine->ended||!initialized||!shader->initialized) return;
	shader->use();
	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
#pragma endregion// Texture

#pragma region Renderers
Renderer::Renderer(Engine* _engine, Shader* _shader) : Object(_engine), shader(_shader), VAO(0), VBO(0), EBO(0) {
	if(!initialized||!shader->initialized) { initialized=false; return; }
	engine->sub_delete(this);
}
void Renderer::draw() {
	if(engine->ended||!initialized) return;
}
void Renderer::on_delete() {
	if(engine->ended||!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
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
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _rotAxis, float _rotAngle) :
	Renderer(_engine, _shader), position(_position), rotAxis(_rotAxis), rotAngle(_rotAngle) {
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
void CubeRenderer::draw() {
	if(engine->ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	shader->bindTextures();
	shader->setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

bool AABBOverlap(Vector2 aPos, Vector2 aSize, Vector2 bPos, Vector2 bSize) {
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
bool Renderer2D::shouldDraw(Vector2 viewer, Vector2 viewRange) {
	return true;
}
bool Renderer2D::shouldDraw(Vector2 viewer, float viewRange) {
	return shouldDraw(viewer, Vector2(viewRange, viewRange));
}

float quadvertices[]={
	0.5f, 0.5f, -1.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -1.0f, 1.0f, 0.0f
};
int quadindices[]={
	0, 1, 3,
	1, 2, 3
};
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale, float _zIndex, float _rotAngle) :
	Renderer2D(_engine, _shader, _position, _scale), position(_position), scale(_scale), zIndex(_zIndex), rotAngle(_rotAngle) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadvertices), quadvertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale) : SpriteRenderer(_engine, _shader, _position, _scale, 0.0f, 0.0f) {}
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, Vector2 _scale, float _zIndex) : SpriteRenderer(_engine, _shader, _position, _scale, _zIndex, 0.0f) {}
void SpriteRenderer::draw() {
	if(engine->ended||!initialized) return;
	Mat4x4 model=scaleMat(Vector3(scale, 1.0f))*axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(Vector3(position, zIndex-100));
	shader->bindTextures();
	shader->setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
bool SpriteRenderer::shouldDraw(Vector2 viewer, Vector2 viewRange) {
	return AABBOverlap(position, scale, viewer, viewRange);
}

struct Character {
	unsigned int TextureID=0;// ID handle of the glyph texture
	Vector2   Size;		     // Size of glyph
	Vector2   Bearing;       // Offset from baseline to left/top of glyph
	unsigned int Advance=0;  // Offset to advance to next glyph
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
		Character character={
			texture,
			Vector2((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows),
			Vector2((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top),
			(unsigned int)face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	characterMapInitialized=true;
	return 1;
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector2 _position, float _scale, Vector3 _color) :
	Renderer2D(_engine, _shader), text(_text), position(_position), scale(_scale), color(_color) {
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
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*5, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindices), quadindices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader->setFloat("text", 0);
}
void TextRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->setFloat3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	float x=position.x;
	float y=position.y;
	std::string::const_iterator c;
	for(c=text.begin(); c!=text.end(); c++) {
		if(*c=='\n') { x=position.x;y-=(8*scale+1); continue; }
		Character ch=Characters[*c];
		float xpos=x+ch.Bearing.x*scale;
		float ypos=y-(ch.Size.y-ch.Bearing.y)*scale;
		float w=ch.Size.x*scale;
		float h=ch.Size.y*scale;
		// update VBO for each character
		float vertices[4][5]={
			{ xpos+w, ypos+h, -1.0f, 1.0f, 0.0f },
			{ xpos, ypos+h, -1.0f, 0.0f, 0.0f },
			{ xpos, ypos, -1.0f, 0.0f, 1.0f },
			{ xpos+w, ypos, -1.0f, 1.0f, 1.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x+=(ch.Advance>>6)*scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position, bool _loop) :
	Renderer2D(_engine, _shader), positions(_positions), width(_width), position(_position), loop(_loop) {
	if(!initialized) return;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	float leftMostX=std::numeric_limits<float>::max();
	float rightMostX=std::numeric_limits<float>::lowest();
	float bottomMostY=std::numeric_limits<float>::max();
	float topMostY=std::numeric_limits<float>::lowest();
	std::vector<float> verts;
	for(Vector2 pos : positions) {
		if(pos.x<leftMostX) leftMostX=pos.x;
		else if(pos.x>rightMostX) rightMostX=pos.x;
		if(pos.y<bottomMostY) bottomMostY=pos.y;
		else if(pos.y>topMostY) topMostY=pos.y;
		verts.push_back(pos.x);
		verts.push_back(pos.y);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
		verts.push_back(0.0f);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_STATIC_DRAW);// fill VBO buffer with vertex data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if(positions.size()>0) {
		boundingBoxPos=Vector2(rightMostX+leftMostX, topMostY+bottomMostY)/2.0f;
		boundingBoxSize=Vector2(rightMostX-leftMostX, topMostY-bottomMostY);
	}
}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width) : LineRenderer(_engine, _shader, _positions, _width, Vector2(0.0f, 0.0f), false) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, bool _loop) : LineRenderer(_engine, _shader, _positions, _width, Vector2(0.0f, 0.0f), _loop) {}
LineRenderer::LineRenderer(Engine* _engine, Shader* _shader, std::vector<Vector2> _positions, float _width, Vector2 _position) : LineRenderer(_engine, _shader, _positions, _width, _position, false) {}
void LineRenderer::draw() {
	if(engine->ended||!initialized) return;
	shader->bindTextures();
	shader->setMat4x4("model", translate(Vector3(position, 0.0f)));
	glBindVertexArray(VAO);
	glLineWidth(width);
	glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, positions.size());
}
bool LineRenderer::shouldDraw(Vector2 viewer, Vector2 viewRange) {
	return AABBOverlap(position+boundingBoxPos, boundingBoxSize, viewer, viewRange);
}
#pragma endregion// Renderers

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
