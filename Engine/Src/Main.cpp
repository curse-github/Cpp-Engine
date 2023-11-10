#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"

class FpsTracker : Object {
	protected:
	double lastFrames[1000]={};
	FpsTracker* self=nullptr;
	public:
	int avgFps=0;
	int highFps=0;
	int lowFps=0;
	float frameTime=0;
	FpsTracker() : Object() {}
	FpsTracker(Engine* _engine) : Object(_engine) { sub_loop(); self=this; }
	void on_loop(double delta) {
		for(int i=1; i<1000; i++) { lastFrames[i-1]=lastFrames[i]; }// move values back
		lastFrames[999]=delta;// put delta at the end
		double sum=0.0f;
		for(int i=0; i<1000; i++) { sum+=lastFrames[i]; }// sum values
		avgFps=(int)(1000.0f/sum+0.5);
		frameTime=sum;//1000.0f*1000.0f;

		highFps=0;
		lowFps=100000;
		for(int i=0; i<1000; i++) {
			if((1/lastFrames[i])>highFps) highFps=((int)(1/lastFrames[i])+0.5);
			if((1/lastFrames[i])<lowFps) lowFps=((int)(1/lastFrames[i])+0.5);
		}
	}
	int getAvgFps() {
		return self->avgFps;
	}
	int getHighFps() {
		return self->highFps;
	}
	int getLowFps() {
		return self->lowFps;
	}
	float getFrameTime() {
		return self->frameTime;
	}
};

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
Shader textShader;
FreeCam cam;
OrthoCam cam2;
vector<CubeRenderer> cubes;
TextRenderer textRend;
FpsTracker tracker;
int main(int argc, char** argv) {
	// setup engine
	engine=Engine(Vector2(1920, 1080), "Game!", false);
	glfwSetWindowUserPointer(engine.window, &engine);
	// setup fps tracker
	tracker=FpsTracker(&engine);
	// setup textures
	Texture tex=Texture(&engine, "Resources/awesomeface.png");
	Texture tex2=Texture(&engine, "Resources/container.jpg");
	// setup shaders
	shader=Shader(&engine, "Shaders/vs.glsl", "Shaders/twoTexFrag.glsl");
	shader.setTexture("texture1", tex, 0);
	shader.setTexture("texture2", tex2, 1);
	shader.setFloat("mixVal", 0.50);
	shader2=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	shader2.setTexture("_texture", tex, 0);
	shader3=Shader(&engine, "Shaders/vs.glsl", "Shaders/colorFrag.glsl");
	shader3.setFloat4("color", Vector4(0.5f, 0.0f, 1.0f, 1.0f));
	if(!engine.initialized||!shader.initialized||!shader2.initialized||!shader3.initialized||engine.ended) return 0;
	// setup cameras
	cam=FreeCam(&engine, engine.screenSize.x/engine.screenSize.y, Vector3(0, 0, 3), Vector3(0, 0, -1), Vector3(0, 1, 0));
	// create cubes at "cubePositions"
	for(int i=0; i<10; i++) {
		if(i%3==0) cubes.push_back(CubeRenderer(&engine, &shader, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		else if(i%3==1) cubes.push_back(CubeRenderer(&engine, &shader2, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		else cubes.push_back(CubeRenderer(&engine, &shader3, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
	}
	//setup text renderer
	textShader=Shader(&engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	textShader.setFloat("text", 0);
	cam2=OrthoCam(&engine, Vector2(400.0f, 300.0f), Vector2(800.0f, 600.0f));
	cam2.set(&textShader);
	textRend=TextRenderer(&engine, &textShader, "");
	// run main loop
	engine.onLoop.push_back(Loop);
	engine.Loop();
	return 1;
}
void Loop(double delta) {
	//set matricies on shaders
	cam.set(&shader);
	cam.set(&shader2);
	cam.set(&shader3);
	//draw cubes
	for(int i=0; i<cubes.size(); i++) {
		cubes[i].draw();
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	textRend.draw("Fps Avg: "+to_string(tracker.getAvgFps())+", high: "+to_string(tracker.getHighFps())+", low: "+to_string(tracker.getLowFps()), 1.0f, 16.0f, 0.5f, Vector3(1.0f, 1.0f, 1.0f));
	textRend.draw("ms: "+to_string(tracker.getFrameTime()), 1.0f, 1.0f, 0.5f, Vector3(1.0f, 1.0f, 1.0f));
}