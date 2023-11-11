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
		for(unsigned int i=1; i<1000; i++) { lastFrames[i-1]=lastFrames[i]; }// move values back
		lastFrames[999]=delta;// put delta at the end
		double sum=0.0f;
		for(unsigned int i=0; i<1000; i++) { sum+=lastFrames[i]; }// sum values
		avgFps=(int)(1000.0f/sum+0.5);
		frameTime=(float)sum;//1000.0f*1000.0f;

		highFps=0;
		lowFps=100000;
		for(unsigned int i=0; i<1000; i++) {
			if((1/lastFrames[i])>highFps) highFps=(int)((1/lastFrames[i])+0.5);
			if((1/lastFrames[i])<lowFps) lowFps=(int)((1/lastFrames[i])+0.5);
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
void on_delete();

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
FpsTracker tracker;
Shader shader;
Shader shader2;
Shader shader3;
Shader textShader;
FreeCam cam;
OrthoCam uiCam;
vector<Renderer*> sceneRenderers;
vector<Renderer*> transparentObjects;
vector<TextRenderer*> debugText;
int main(int argc, char** argv) {
	// setup engine
	engine=Engine(Vector2(1920, 1080), "Game!", false);
	if(!engine.initialized||engine.ended) {
		Log("Engine failed to init");
		return 0;
	}
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
	textShader=Shader(&engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	textShader.setFloat("text", 0);
	if(!engine.initialized||engine.ended||!shader.initialized||!shader2.initialized||!shader3.initialized||!textShader.initialized) {
		Log("Shaders failed to init");
		engine.Delete();
		return 0;
	}
	// setup cameras
	cam=FreeCam(&engine, engine.screenSize.x/engine.screenSize.y, Vector3(0, 0, 3), Vector3(0, 0, -1), Vector3(0, 1, 0));
	uiCam=OrthoCam(&engine, Vector2(400.0f, 300.0f), Vector2(800.0f, 600.0f)); uiCam.set(&textShader);
	if(!engine.initialized||engine.ended||!cam.initialized||!uiCam.initialized) {
		Log("Cameras failed to init");
		engine.Delete();
		return 0;
	}
	// create cubes at "cubePositions"
	for(int i=0; i<10; i++) {
		if(i%3==0) transparentObjects.push_back(new CubeRenderer(&engine, &shader, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		else if(i%3==1) transparentObjects.push_back(new CubeRenderer(&engine, &shader2, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
		else sceneRenderers.push_back(new CubeRenderer(&engine, &shader3, cubePositions[i], Vector3(1.0f, 0.3f, 0.5f), i*20.0f));
	}
	// setup text renderers
	debugText.push_back(new TextRenderer(&engine, &textShader, "ms: ",Vector2(1.0f,1.0f),0.5f,Vector3(1.0f,1.0f,1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "Fps Avg: ",Vector2(1.0f,16.0f),0.5f,Vector3(1.0f,1.0f,1.0f)));
	if(!engine.initialized||engine.ended){//||!characterMapInitialized) {
		Log("Fonts failed to init");
		engine.Delete();
		return 0;
	}
	// run main loop
	engine.onLoop.push_back(Loop);
	engine.onDelete.push_back(on_delete);
	Log("Engine initialized successfully");
	engine.Loop();
	return 1;
}
void Loop(double delta) {
	// set debug text
	debugText[1]->text="Fps Avg: "+to_string(tracker.getAvgFps())+", high: "+to_string(tracker.getHighFps())+", low: "+to_string(tracker.getLowFps());
	debugText[0]->text="ms: "+to_string(tracker.getFrameTime());
	// set matricies on shaders
	cam.set(&shader);
	cam.set(&shader2);
	cam.set(&shader3);
	//draw scene
	for(Renderer* rend : sceneRenderers) rend->draw();
	for(Renderer* rend : transparentObjects) rend->draw();
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(TextRenderer* rend : debugText) rend->draw();
}
void on_delete() {
	for(unsigned int i=0; i<sceneRenderers.size(); i++) delete sceneRenderers[i];
	for(unsigned int i=0; i<debugText.size(); i++) delete debugText[i];
}