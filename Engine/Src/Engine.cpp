#include "Engine.h"
#include <algorithm>
#include <fstream>
#include <stb_image.h>
#include <freetype/freetype.h>
//#include <map>
#define clamp(a,b,c) max(b,min(a,c))

#pragma region Engine
Engine::Engine(Vector2 size, const char* title, bool vsync) {
	initialized=true;
	if(!glfwInit()) {
		ended=true;
		//Log("");//error
		return;
	}

	window=glfwCreateWindow((int)size.x, (int)size.y, title, NULL, NULL);
	if(!window) {
		glfwTerminate();
		ended=true;
		//Log("");//error
		return;
	}
	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ended=true;
		glfwDestroyWindow(window);
		glfwTerminate();
		//Log("");//error
		return;
	}
	glViewport(0, 0, (int)size.x, (int)size.y);
	glfwSwapInterval((int)vsync);// V-Sync: 1=on, 0=off

	glfwSetWindowUserPointer(window, this);
	glfwSetErrorCallback(engine_on_error);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_resize(window, width, height);});
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_key(window, key, scancode, action, mods);});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_scroll(window, xoffset, yoffset);});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double mouseX, double mouseY) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse(window, mouseX, mouseY);});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse_button(window, button, action, mods);});
	glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) {static_cast<Engine*>(glfwGetWindowUserPointer(window))->on_mouse_enter(window, entered);});

	screenSize=size;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
}
void Engine::Loop() {
	if(ended||!initialized) return;
	double lastFrameTime=0.0f;
	while(!glfwWindowShouldClose(window)) {
		double delta=glfwGetTime()-lastFrameTime;
		lastFrameTime=glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		for_each(onLoop.begin(), onLoop.end(), [delta](onloopfun callback) {
			callback(delta);
		});
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
	for_each(onDelete.begin(), onDelete.end(), [](ondeletefun callback) {
		callback();
	});
	glfwDestroyWindow(window);
	glfwTerminate();
}

void engine_on_error(int error, const char* description) {
	Log("Error: "+string(description));
#ifdef _DEBUG
	__debugbreak();
#endif
}
void Engine::on_resize(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height); screenSize=Vector2((float)width, (float)height);
	for_each(onResize.begin(), onResize.end(), [window, width, height](onresizefun callback) {
		callback(window, width, height);
	});
}
void Engine::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	for_each(onKey.begin(), onKey.end(), [window, key, scancode, action, mods](onkeyfun callback) {
		callback(window, key, scancode, action, mods);
	});
}
void Engine::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {
	for_each(onScroll.begin(), onScroll.end(), [window, xoffset, yoffset](onscrollfun callback) {
		callback(window, xoffset, yoffset);
	});
}
void Engine::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {
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
	for_each(onMouseDelta.begin(), onMouseDelta.end(), [window, deltaX, deltaY](onmousedeltafun callback) {
		callback(window, deltaX, deltaY);
	});
}
void Engine::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	for_each(onMouseButton.begin(), onMouseButton.end(), [window, button, action, mods](onmousebuttonfun callback) {
		callback(window, button, action, mods);
	});
}
void Engine::on_mouse_enter(GLFWwindow* window, int entered) {
	for_each(onMouseEnter.begin(), onMouseEnter.end(), [window, entered](onmouseenterfun callback) {
		callback(window, entered);
	});
}
#pragma endregion// Engine

#pragma region Object
Object::Object(Engine* _engine) : engine(_engine) { initialized=true; }
void Object::sub_resize() {
	engine->onResize.push_back([=](GLFWwindow* window, int width, int height) {
		this->on_resize(window, width, height);
	});
}
void Object::sub_key() {
	engine->onKey.push_back([=](GLFWwindow* window, int key, int scancode, int action, int mods) {
		this->on_key(window, key, scancode, action, mods);
	});
}
void Object::sub_scroll() {
	engine->onScroll.push_back([=](GLFWwindow* window, double xoffset, double yoffset) {
		this->on_scroll(window, xoffset, yoffset);
	});
}
void Object::sub_mouse() {
	engine->onMouse.push_back([=](GLFWwindow* window, double mouseX, double mouseY) {
		this->on_mouse(window, mouseX, mouseY);
	});
}
void Object::sub_mouse_delta() {
	engine->onMouseDelta.push_back([=](GLFWwindow* window, float deltaX, float deltaY) {
		this->on_mouse_delta(window, deltaX, deltaY);
	});
}
void Object::sub_mouse_button() {
	engine->onMouseButton.push_back([=](GLFWwindow* window, int button, int action, int mods) {
		this->on_mouse_button(window, button, action, mods);
	});
}
void Object::sub_mouse_enter() {
	engine->onMouseEnter.push_back([=](GLFWwindow* window, int entered) {
		this->on_mouse_enter(window, entered);
	});
}
void Object::sub_delete() {
	engine->onDelete.push_back([=]() {
		this->on_delete();
	});
}
void Object::sub_loop() {
	engine->onLoop.push_back([=](double delta) {
		this->on_loop(delta);
	});
}

void Object::on_resize(GLFWwindow* window, int width, int height) {}
void Object::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) { Log("test"); }
void Object::on_scroll(GLFWwindow* window, double xoffset, double yoffset) {}
void Object::on_mouse(GLFWwindow* window, double mouseX, double mouseY) {}
void Object::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) { Log("test"); }
void Object::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {}
void Object::on_mouse_enter(GLFWwindow* window, int entered) {}
void Object::on_delete() {}
void Object::on_loop(double delta) {}
#pragma endregion// Object

#pragma region Shader
void FsReadDiskFile(string* content, const string& filePath) {
	ifstream fileStream(filePath, std::ios::in);
	if(!fileStream.is_open()) { return; }
	string line;
	while(getline(fileStream, line)) {
		*content+=line+"\n";
	}
	fileStream.close();
}
Shader::Shader(Engine* _engine, string vertexPath, string fragmentPath) : Object(_engine) {
	if((*engine).ended||!initialized) { initialized=false;return; }
	// read vertex shader from file
	string vertexShaderSourceStr;
	FsReadDiskFile(&vertexShaderSourceStr, vertexPath);
	if(sizeof(vertexShaderSourceStr)==0) {
		Log("Error reading "+vertexPath+".");
		//Log("");//error
		(*engine).Delete();
		return;
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
		Log("Error initializing vertex shader.");
		glDeleteShader(vertexShader);
		//Log("");//error
		(*engine).Delete();
		return;

	}
	// read fragment shader from file
	string fragmentShaderSourceStr;
	FsReadDiskFile(&fragmentShaderSourceStr, fragmentPath);
	if(sizeof(fragmentShaderSourceStr)==0) {
		Log("Error reading "+fragmentPath+".");
		//Log("");//error
		(*engine).Delete();
		return;

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
		Log("Error initializing fragment shader.");
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		//Log("");//error
		(*engine).Delete();
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
		Log("Error initializing shader program.");
		on_delete();
		//Log("");//error
		(*engine).Delete();
		return;

	}

	sub_delete();
}
void Shader::use() {
	if((*engine).ended||!initialized) return;
	glUseProgram(program);
}
void Shader::on_delete() {
	if(!initialized) return;
	glDeleteProgram(program);
}
// uniform utility functions
void Shader::setBool(const std::string& name, bool value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, (bool)value);
}
void Shader::setInt(const std::string& name, int value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, value);
}
void Shader::setFloat(const std::string& name, float value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1f(uniformLocation, value);
}
void Shader::setFloat2(const std::string& name, Vector2 value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform2f(uniformLocation, value.x, value.y);
}
void Shader::setFloat3(const std::string& name, Vector3 value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}
void Shader::setFloat4(const std::string& name, Vector4 value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
}
void Shader::setMat4x4(const std::string& name, Mat4x4 value) {
	if((*engine).ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value.values[0]);
}
void Shader::setTexture(const std::string& name, Texture tex, unsigned int location) {
	setInt(name, location);
	textures.push_back(tex);
	textureLocations.push_back(location);
}
void Shader::bindTextures() {
	for(size_t i=0; i<min(textures.size(), textureLocations.size()); i++) {
		textures[i].Bind(this, textureLocations[i]);
	}
}
#pragma endregion// Shader

#pragma region Camera
// save reference to object as it is in the contructor to be used in the set function
// similar effect as glfwSetWindowUserPointer in the main function
Camera::Camera(Engine* _engine) : Object(_engine), projection(Mat4x4()), view(Mat4x4()) { self=this; }
void Camera::update() {}
void Camera::set(Shader* shader) {
	if((*engine).ended||!initialized) return;
	(*shader).setMat4x4("projection", self->projection);
	(*shader).setMat4x4("view", self->view);
}

LookAtCam::LookAtCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _focus) :
	Camera(_engine), aspect(_aspect), position(_position), focus(_focus) {
	if((*engine).ended||!initialized) { initialized=false;return; }
	update();
}
void LookAtCam::update() {
	if((*engine).ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
}

FreeCam::FreeCam(Engine* _engine, float _aspect, Vector3 _position, Vector3 _forward, Vector3 _up) :
	Camera(_engine), aspect(_aspect), position(_position), forward(_forward), up(_up) {
	glfwSetInputMode((*engine).window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Object::sub_key();
	Object::sub_scroll();
	Object::sub_mouse_delta();
	Object::sub_loop();
}
void FreeCam::update() {
	if((*engine).ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	view=lookAt(position, position+forward, up);
	Camera::update();
}
void FreeCam::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if((*engine).ended||!initialized) return;
	if(key==GLFW_KEY_ESCAPE&&action==GLFW_PRESS) {
		paused=!paused;
		glfwSetInputMode(window, GLFW_CURSOR, paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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
	if((*engine).ended||!initialized) return;
	fov-=(float)yoffset;
	fov=clamp(fov, 1.0f, 90.0f);
	update();
}
void FreeCam::on_mouse_delta(GLFWwindow* window, float deltaX, float deltaY) {
	if((*engine).ended||!initialized) return;
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
	initialized=true;
	update();
}
void OrthoCam::update() {
	if(!initialized) return;
	projection=ortho(-size.x/2.0f, size.x/2.0f, -size.y/2.0f, size.y/2.0f, 0.01f, 100.0f);
	view=translate(Vector3(-position, 0));
}
#pragma endregion// Camera

#pragma region Texture
int load_texture(unsigned int* texture, string path) {
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data=stbi_load(path.c_str(), &width, &height, &nrChannels, 0);//read raw image data from file
	if(data) {
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);// bind texture so that following code will assign the texture
											   // texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// turn byte data into texture
		unsigned int channelsEnum=((nrChannels==3) ? GL_RGB : GL_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, channelsEnum, width, height, 0, channelsEnum, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);// free the memory holding the image data
		return 1;
	} else {
		return 0;
	}
}
Texture::Texture(Engine* _engine, string _path) :
	Object(_engine), path(_path) {
	if(!load_texture(&ID, path)) {
		Log("Error initializing texture \""+path+"\".");
		initialized=false;
		//Log("");//error
		(*engine).Delete();
		return;
	}
}
void Texture::Bind(Shader* shader, unsigned int location) {
	(*shader).use();
	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(GL_TEXTURE_2D, ID);
}
#pragma endregion// Texture

#pragma region Renderers
Renderer::Renderer(Engine* _engine, Shader* _shader) : Object(_engine), shader(_shader), VAO(0), VBO(0), EBO(0) {
	sub_delete();
}
void Renderer::draw() {}
void Renderer::on_delete() {
	if(!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
float cubevertices[]={
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f
};
CubeRenderer::CubeRenderer(Engine* _engine, Shader* _shader, Vector3 _position, Vector3 _rotAxis, float _rotAngle) :
	Renderer(_engine, _shader), position(_position), rotAxis(_rotAxis), rotAngle(_rotAngle) {
	if((*engine).ended) { initialized=false;return; }
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubevertices), cubevertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);// fill EBO buffer with index data
	// start, length, type, ?, total size, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
}
void CubeRenderer::draw() {
	if((*engine).ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	(*shader).bindTextures();
	(*shader).use();
	(*shader).setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

float quadvertices[]={
	0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};
int quadindices[]={
	0, 1, 3,
	1, 2, 3
};
SpriteRenderer::SpriteRenderer(Engine* _engine, Shader* _shader, Vector2 _position, float _rotAngle) :
	Renderer(_engine, _shader), position(_position), rotAngle(_rotAngle) {
	if((*engine).ended) { initialized=false;return; }
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
}
void SpriteRenderer::draw() {
	if((*engine).ended||!initialized) return;
	Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(Vector3(position, 0.0f));
	(*shader).setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	Vector2   Size;		     // Size of glyph
	Vector2   Bearing;       // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;
bool characterMapInitialized=false;
int initCharacterMap() {
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) return 0;
	FT_Face face;
	if(FT_New_Face(ft, "Fonts/MonocraftBetterBrackets.ttf", 0, &face)) return 0;
	FT_Set_Pixel_Sizes(face, 0, 30);
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
	return 1;
}
TextRenderer::TextRenderer(Engine* _engine, Shader* _shader, std::string _text, Vector2 _position, float _scale, Vector3 _color) :
	Renderer(_engine, _shader), text(_text), position(_position), scale(_scale), color(_color) {
	if((*engine).ended) { initialized=false;return; }
	if(!characterMapInitialized) {
		if(!initCharacterMap()) {
			initialized=false;
			//Log("");//error
			(*engine).Delete();
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
}
void TextRenderer::draw() {
	if((*engine).ended||!initialized) return;
	(*shader).setFloat3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	float x=position.x;
	std::string::const_iterator c;
	for(c=text.begin(); c!=text.end(); c++) {
		Character ch=Characters[*c];
		float xpos=x+ch.Bearing.x*scale;
		float ypos=position.y-(ch.Size.y-ch.Bearing.y)*scale;
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
#pragma endregion// Renderers