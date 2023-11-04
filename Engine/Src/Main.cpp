#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"

void Loop(double delta);
Vector3 cubePositions[10]={
	Vector3(0.0f, 0.0f, 0.0f),
	Vector3(2.0f, 5.0f, -15.0f),
	Vector3(-1.5f, -2.2f, -2.5f),
	Vector3(-3.8f, -2.0f, -12.3f),
	Vector3(2.4f, -0.4f, -3.5f),
	Vector3(-1.7f, 3.0f, -7.5f),
	Vector3(1.3f, -2.0f, -2.5f),
	Vector3(1.5f, 2.0f, -2.5f),
	Vector3(1.5f, 0.2f, -1.5f),
	Vector3(-1.3f, 1.0f, -1.5f)
};

Engine engine;
Shader shader;
Shader shader2;
Shader shader3;
Camera cam;
Camera cam2;
Camera cam3;
vector<CubeRenderer> cubes;
int main(int argc, char** argv) {
	// setup engine
	engine=Engine(Vector2(1920, 1080), "FPS:", true);
	glfwSetWindowUserPointer(engine.window, &engine);
	// setup textures
	Texture tex=Texture(&engine, "Resources/awesomeface.png");
	Texture tex2=Texture(&engine, "Resources/container.jpg");
	// setup shaders
	shader=Shader(&engine, "Shaders/vs.glsl", "Shaders/twoTexFrag.glsl");
	shader.addTexture(tex, "texture1", 0);
	shader.addTexture(tex2, "texture2", 1);
	shader.setFloat("mixVal", 0.50);
	shader2=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	shader2.addTexture(tex2, "_texture", 0);
	shader3=Shader(&engine, "Shaders/vs.glsl", "Shaders/colorFrag.glsl");
	shader3.setFloat4("color", Vector4(0.5f, 0.0f, 1.0f, 1.0f));
	if(!engine.initialized||!shader.initialized||!shader2.initialized||!shader3.initialized||engine.ended) return 0;
	// setup cameras, need better solution
	cam=FreeCam(&engine, &shader, engine.screenSize.x/engine.screenSize.y, Vector3(0, 0, 3), Vector3(0, 0, -1), Vector3(0, 1, 0));
	cam2=FreeCam(&engine, &shader2, engine.screenSize.x/engine.screenSize.y, Vector3(0, 0, 3), Vector3(0, 0, -1), Vector3(0, 1, 0));
	cam3=FreeCam(&engine, &shader3, engine.screenSize.x/engine.screenSize.y, Vector3(0, 0, 3), Vector3(0, 0, -1), Vector3(0, 1, 0));
	// create cubes at "cubePositions"
	for(int i=0; i<10; i++) {
		Log(i%3);
		if(i%3==0) cubes.push_back(CubeRenderer(&engine, &shader, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		if(i%3==1) cubes.push_back(CubeRenderer(&engine, &shader2, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		if(i%3==2) cubes.push_back(CubeRenderer(&engine, &shader3, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
	}
	if(engine.ended) return 0;
	// run main loop
	engine.onLoop.push_back(Loop);
	engine.Loop();
	return 1;
}
void Loop(double delta) {
	//draw cubes
	for(int i=0; i<10; i++) {
		cubes[i].draw();
	}
}