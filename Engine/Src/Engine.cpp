#include "Engine.h"
#include <algorithm>
#include <fstream>
#pragma region Engine
Engine::Engine(Vector2 size, const char* title, bool vsync) {
	initialized=true;
	if(!glfwInit()) { ended=true; return; }

	window=glfwCreateWindow((int)size.x, (int)size.y, title, NULL, NULL);
	if(!window) { ended=true; glfwTerminate(); return; }
	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ended=true;
		glfwDestroyWindow(window);
		glfwTerminate();
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
}
void Engine::Loop() {
	if(ended||!initialized) return;
	double lastFrameTime=0.0f;
	while(!glfwWindowShouldClose(window)) {
		double delta=glfwGetTime()-lastFrameTime;
		lastFrameTime=glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
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
	Log("Error: "+string(description)); __debugbreak();
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
	if((deltaX!=0)||(deltaY!=0)) on_mouse_delta(window, deltaX, deltaY);
	for_each(onMouse.begin(), onMouse.end(), [window, mouseX, mouseY](onmousefun callback) {
		callback(window, mouseX, mouseY);
	});
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
Object::Object(Engine _engine) : engine(_engine) { initialized=true; }
void Object::sub_resize() {
	engine.onResize.push_back([this](GLFWwindow* window, int width, int height) {
		this->on_resize(window, width, height);
	});
}
void Object::sub_key() {
	engine.onKey.push_back([this](GLFWwindow* window, int key, int scancode, int action, int mods) {
		this->on_key(window, key, scancode, action, mods);
	});
}
void Object::sub_scroll() {
	engine.onScroll.push_back([this](GLFWwindow* window, double xoffset, double yoffset) {
		this->on_scroll(window, xoffset, yoffset);
	});
}
void Object::sub_mouse() {
	engine.onMouse.push_back([this](GLFWwindow* window, double mouseX, double mouseY) {
		this->on_mouse(window, mouseX, mouseY);
	});
}
void Object::sub_mouse_delta() {
	engine.onMouseDelta.push_back([this](GLFWwindow* window, float deltaX, float deltaY) {
		this->on_mouse_delta(window, deltaX, deltaY);
	});
}
void Object::sub_mouse_button() {
	engine.onMouseButton.push_back([this](GLFWwindow* window, int button, int action, int mods) {
		this->on_mouse_button(window, button, action, mods);
	});
}
void Object::sub_mouse_enter() {
	engine.onMouseEnter.push_back([this](GLFWwindow* window, int entered) {
		this->on_mouse_enter(window, entered);
	});
}
void Object::sub_delete() {
	engine.onDelete.push_back([this]() {
		this->on_delete();
	});
}
void Object::sub_loop() {
	engine.onLoop.push_back([this](double delta) {
		this->on_loop(delta);
	});
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
void FsReadDiskFile(string* content, const string& filePath) {
	ifstream fileStream(filePath, std::ios::in);
	if(!fileStream.is_open()) { return; }
	string line;
	while(getline(fileStream, line)) {
		*content+=line+"\n";
	}
	fileStream.close();
}
Shader::Shader(Engine _engine, string vertexPath, string fragmentPath) : Object(_engine) {
	if(engine.ended||!initialized) { initialized=false;return; }
	// read vertex shader from file
	string vertexShaderSourceStr;
	FsReadDiskFile(&vertexShaderSourceStr, vertexPath);
	if(sizeof(vertexShaderSourceStr)==0) {
		Log("Error reading "+vertexPath+".");
		engine.Delete();
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
		engine.Delete();
		return;
	}
	// read fragment shader from file
	string fragmentShaderSourceStr;
	FsReadDiskFile(&fragmentShaderSourceStr, fragmentPath);
	if(sizeof(fragmentShaderSourceStr)==0) {
		Log("Error reading "+fragmentPath+".");
		engine.Delete();
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
		engine.Delete();
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
		engine.Delete();
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	Object::sub_delete();
}
void Shader::use() {
	if(engine.ended||!initialized) return;
	glUseProgram(program);
}
void Shader::on_delete() {
	if(!initialized) return;
	glDeleteProgram(program);
}
// uniform utility functions
void Shader::setBool(const std::string& name, bool value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, (bool)value);
}
void Shader::setInt(const std::string& name, int value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1i(uniformLocation, value);
}
void Shader::setFloat(const std::string& name, float value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform1f(uniformLocation, value);
}
void Shader::setFloat2(const std::string& name, Vector2 value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform2f(uniformLocation, value.x, value.y);
}
void Shader::setFloat3(const std::string& name, Vector3 value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}
void Shader::setFloat4(const std::string& name, Vector4 value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
}
void Shader::setMat4x4(const std::string& name, Mat4x4 value) {
	if(engine.ended||!initialized) return;
	use();
	int uniformLocation=glGetUniformLocation(program, name.c_str());
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value.values[0]);
}
#pragma endregion// Shader

#pragma region Camera
Camera::Camera(Engine _engine, Shader _shader) : Object(_engine), shader(_shader), projection(Mat4x4()), view(Mat4x4()) {}
void Camera::updateProjection() {
	if(engine.ended||!initialized) return;
	shader.setMat4x4("projection", projection);
}
void Camera::updateView() {
	if(engine.ended||!initialized) return;
	shader.setMat4x4("view", view);
}

LookAtCam::LookAtCam(Engine _engine, Shader _shader, float _aspect, Vector3 _position, Vector3 _focus) :
	Camera(_engine, _shader), position(_position), aspect(_aspect), focus(_focus) {
	if(engine.ended||!initialized) { initialized=false;return; }
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	Camera::updateProjection();
}
void LookAtCam::updateProjection() {
	if(engine.ended||!initialized) return;
	projection=perspective(deg_to_rad(fov), aspect, 0.1f, 100.0f);
	Camera::updateProjection();
}
void LookAtCam::updateView() {
	if(engine.ended||!initialized) return;
	view=lookAt(position, focus, Vector3(0.0f, 1.0f, 0.0f));
	Camera::updateView();
}
#pragma endregion// Camera

#pragma region Renderers
float vertices[]={
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
CubeRenderer::CubeRenderer(Engine _engine, Shader _shader, Vector3 _position, Vector3 _rotAxis, float _rotAngle) :
	Object(_engine), shader(_shader), position(_position), rotAxis(_rotAxis), rotAngle(_rotAngle) {
	if(engine.ended||!initialized) { initialized=false;return; }

	shader.use();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);// bind buffer so that following code will assign the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);// fill VBO buffer with vertex data
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);// bind buffer so that following code will assign the EBO buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);// fill EBO buffer with index data

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);// get vertex position data
	glEnableVertexAttribArray(0);// bind data above to (location = 1) in vertex shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));// get vertex uv data
	glEnableVertexAttribArray(1);// bind data above to (location = 2) in vertex shader
}
void CubeRenderer::draw() {
	if(engine.ended||!initialized) return;
	//Mat4x4 model=axisRotMat(rotAxis, deg_to_rad(rotAngle))*translate(position);
	Mat4x4 model=Mat4x4();
	shader.use();
	shader.setMat4x4("model", model);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
void CubeRenderer::on_delete() {
	if(!initialized) return;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
}
#pragma endregion// Renderers