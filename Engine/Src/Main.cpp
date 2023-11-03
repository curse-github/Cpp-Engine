#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"
void test(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Log(key);
	Log(scancode);
	Log(action);
	Log(mods);
}
void Loop(double delta);
Engine engine;
Shader shader;
LookAtCam cam;
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
vector<CubeRenderer> cubes;
int main(int argc, char** argv) {
	engine = Engine(Vector2(1920, 1080),"",true);
	shader = Shader(engine,"Shaders/vs.glsl", "Shaders/fs.glsl");
	if (!engine.initialized||!shader.initialized||engine.ended) return 0;
	cam = LookAtCam(engine,shader,engine.screenSize.x/engine.screenSize.y,Vector3(0,0,3),Vector3(0,0,0));

	for(int i=0; i<10; i++) {
		cubes.push_back(CubeRenderer(engine,shader,cubePositions[i],Vector3(1.0f, 0.3f, 0.5f),deg_to_rad(i*20.0f)));
	}

	engine.onLoop.push_back(Loop);
	engine.Loop();
	return 1;
}
void Loop(double delta) {
	shader.use();
	for(int i=0; i<10; i++) {
		cubes[i].draw();
	}
}