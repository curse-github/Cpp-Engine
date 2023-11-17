#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"

class FpsTracker : Object {
protected:
	double lastFrames[60]={};
	FpsTracker* self=nullptr;
public:
	int avgFps=0;
	int highFps=0;
	int lowFps=0;
	float frameTime=0;
	FpsTracker() : Object() {}
	FpsTracker(Engine* _engine) : Object(_engine) { sub_loop(); self=this; }
	void on_loop(double delta) {
		for(unsigned int i=1; i < 60; i++) { lastFrames[i - 1]=lastFrames[i]; }// move values back
		lastFrames[59]=delta;// put delta at the end
		double sum=0.0f;
		for(unsigned int i=0; i < 60; i++) { sum+=lastFrames[i]; }// sum values
		avgFps=(int)(60.0f / sum + 0.5);
		frameTime=(float)sum;//1000.0f*1000.0f;

		highFps=0;
		lowFps=100000;
		for(unsigned int i=0; i < 60; i++) {
			if((1 / lastFrames[i]) > highFps) highFps=(int)((1 / lastFrames[i]) + 0.5);
			if((1 / lastFrames[i]) < lowFps) lowFps=(int)((1 / lastFrames[i]) + 0.5);
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
void onDelete();

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
Shader textShader;
Shader backgroundShader;
Shader playerShader;
Shader minimapShader;
OrthoCam cam;
OrthoCam uiCam;
vector<Renderer*> sceneRenderers;
vector<Renderer*> uiRenderers;
vector<TextRenderer*> debugText;
SpriteRenderer playerRenderer;
int main(int argc, char** argv) {
	// setup engine
	engine=Engine(Vector2(1920, 1080), "Game!", true);
	if(!engine.initialized || engine.ended) {
		Log("Engine failed to init");
		return 0;
	}
	glfwSetWindowUserPointer(engine.window, &engine);
	// setup fps tracker
	tracker=FpsTracker(&engine);
	// setup cameras
	cam=OrthoCam(&engine, Vector2(0.0f, 0.0f), Vector2(480.0f, 270.0f));
	uiCam=OrthoCam(&engine, Vector2(480.0f, 270.0f), Vector2(960.0f, 540.0f));
	if(engine.ended || !cam.initialized || !uiCam.initialized) {
		Log("Cameras failed to init");
		engine.Delete();
		return 0;
	}
	// setup textures
	Texture backgroundTex(&engine, "Resources/map.png");
	Texture playerTex(&engine, "Resources/ghost.png");
	Texture minimapTex(&engine, "Resources/miniMap.png");
	if(engine.ended || !backgroundTex.initialized || !playerTex.initialized || !minimapTex.initialized) {
		Log("Textures failed to init");
		engine.Delete();
		return 0;
	}
	float minimapScale=0.15f;
	Vector2 minimapSize=Vector2((float)minimapTex.width, (float)minimapTex.height) * minimapScale;
	// setup shaders
	backgroundShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	playerShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	minimapShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	textShader=Shader(&engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	if(engine.ended ||
		!backgroundShader.initialized || !playerShader.initialized ||
		!minimapShader.initialized || !textShader.initialized
		) {
		Log("Shaders failed to init");
		engine.Delete();
		return 0;
	}
	backgroundShader.setTexture("_texture", &backgroundTex, 0);
	playerShader.setTexture("_texture", &playerTex, 0);
	minimapShader.setTexture("_texture", &minimapTex, 0);
	cam.bindShader(&backgroundShader);
	cam.bindShader(&playerShader);
	uiCam.bindShader(&minimapShader);
	uiCam.bindShader(&textShader);
	cam.use();
	uiCam.use();
	// setup player stuff
	playerRenderer=SpriteRenderer(&engine, &playerShader, Vector2(0,0),Vector2(15.0f,15.0f),0.0f);
	uiRenderers.push_back(&playerRenderer);
	// setup text renderers
	debugText.push_back(new TextRenderer(&engine, &textShader, "Time: ", Vector2(1.0f, 1.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "Fps Avg: ", Vector2(1.0f, 17.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "ms: ", Vector2(1.0f, 33.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	if(engine.ended || !characterMapInitialized) {
		Log("Fonts failed to init");
		engine.Delete();
		return 0;
	}
	// setup UI
	uiRenderers.push_back(new SpriteRenderer(&engine, &minimapShader, Vector2(minimapSize.x/2, 540.0f - minimapSize.y/2), minimapSize, 0.0f));// minimap
	// run main loop
	engine.onLoop.push_back(Loop);
	engine.onLoopNames.push_back("Main");
	Log("Engine initialized successfully");
	engine.Loop();
	return 1;
}
void Loop(double delta) {
	// set debug text
	debugText[0]->text="Time: " + to_string(glfwGetTime());
	debugText[1]->text="Fps Avg: " + to_string(tracker.getAvgFps()) + ", high: " + to_string(tracker.getHighFps()) + ", low: " + to_string(tracker.getLowFps());
	debugText[2]->text="ms: " + to_string(tracker.getFrameTime());
	//draw scene
	for(Renderer* rend : sceneRenderers) rend->draw();
	// draw ui
	glClear(GL_DEPTH_BUFFER_BIT);
	for(Renderer* rend : uiRenderers) rend->draw();
	for(TextRenderer* rend : debugText) rend->draw();
}
void onDelete() {
	for(unsigned int i=0; i < sceneRenderers.size(); i++) { delete sceneRenderers[i]; }
	for(unsigned int i=0; i < uiRenderers.size(); i++) { delete uiRenderers[i]; }
	for(unsigned int i=0; i < debugText.size(); i++) { delete debugText[i]; }
}