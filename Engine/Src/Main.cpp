#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"

float mapScale=35.0f;
float spacing=0.05f;
Vector2 offset=Vector2(-26.5f, -4.0f);
float broken_percent=2.5f;
Vector2 map_size=Vector2(49.0f, 34.0f);

float playerSpeed=5.0f;

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
	FpsTracker(Engine* _engine) : Object(_engine) {
		self=this;
		if(!initialized) return;
		sub_loop();
	}
	void on_loop(double delta) override {
		if(engine->ended || !initialized) return;
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
		if(engine->ended || !initialized) return 0;
		return self->avgFps;
	}
	int getHighFps() {
		if(engine->ended || !initialized) return 0;
		return self->highFps;
	}
	int getLowFps() {
		if(engine->ended || !initialized) return 0;
		return self->lowFps;
	}
	float getFrameTime() {
		if(engine->ended || !initialized) return 0;
		return self->frameTime;
	}
};
class PlayerController : public Object {
protected:
	PlayerController* self=nullptr;
	SpriteRenderer* playerRenderer;
	OrthoCam* sceneCam;
public:
	PlayerController() : Object(), playerRenderer(nullptr), sceneCam(nullptr) {}
	PlayerController(Engine* _engine, SpriteRenderer* _playerRenderer, OrthoCam* _sceneCam)
		: Object(_engine), playerRenderer(_playerRenderer), sceneCam(_sceneCam) {
		self=this;
		if(!initialized) return;
		sub_key();
		sub_loop();
		sceneCam->position=playerRenderer->position;
	}
	bool paused=false;
	int inputs[4]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if(engine->ended || !initialized) return;
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			paused=!paused;
			engine->SetCursor(paused?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);
			return;
		}
		if(paused) return;
		if(key == GLFW_KEY_W) inputs[0]=action;
		else if(key == GLFW_KEY_A) inputs[1]=action;
		else if(key == GLFW_KEY_S) inputs[2]=action;
		else if(key == GLFW_KEY_D) inputs[3]=action;
	}
	void on_loop(double delta) {
		if(engine->ended || !initialized) return;
		Vector2 inputVec=Vector2(
			(float)(inputs[3] >= GLFW_PRESS) - (float)(inputs[1] >= GLFW_PRESS),
			(float)(inputs[0] >= GLFW_PRESS) - (float)(inputs[2] >= GLFW_PRESS)
		) * ((float)delta) * mapScale * playerSpeed;
		if(inputVec.x == 0 && inputVec.y == 0) return;
		playerRenderer->position+=inputVec;
		sceneCam->position=playerRenderer->position;
		sceneCam->update();
		sceneCam->use();
	}
};

void Loop(double delta);
void onDelete();

Engine engine;
FpsTracker tracker;
Shader textShader;
Shader backgroundShader;
Shader playerShader;
Shader minimapShader;
OrthoCam* cam;
OrthoCam* uiCam;
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
	cam=new OrthoCam(&engine, Vector2(0.0f, 0.0f), Vector2(480.0f, 270.0f));
	uiCam=new OrthoCam(&engine, Vector2(480.0f, 270.0f), Vector2(960.0f, 540.0f));
	if(engine.ended || !cam->initialized || !uiCam->initialized) {
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
	const float minimapScale=0.15f;
	Vector2 minimapSize=Vector2((float)minimapTex.width, (float)minimapTex.height) * minimapScale;
	// setup shaders
	backgroundShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	playerShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/modulateTexFrag.glsl");
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
	playerShader.setFloat3("color", Vector3(0.35, 0.0, 0.7));// purple
	minimapShader.setTexture("_texture", &minimapTex, 0);
	cam->bindShader(&backgroundShader);
	cam->bindShader(&playerShader);
	uiCam->bindShader(&minimapShader);
	uiCam->bindShader(&textShader);
	cam->use();
	uiCam->use();
	// setup game stuff
	Vector2 fullMapSize=map_size * (1 + spacing) * mapScale;
	SpriteRenderer bgRenderer=SpriteRenderer(&engine, &backgroundShader, fullMapSize / 2.0f + (offset * (1 + spacing) * mapScale), fullMapSize, 0.0f);
	bgRenderer.zIndex=1;
	sceneRenderers.push_back(&bgRenderer);

	playerRenderer=SpriteRenderer(&engine, &playerShader, Vector2(0, 0), Vector2(0.5f, 0.5f) * mapScale, 0.0f);
	PlayerController controller=PlayerController(&engine, &playerRenderer, cam);
	sceneRenderers.push_back(&playerRenderer);
	// setup UI
	uiRenderers.push_back(new SpriteRenderer(&engine, &minimapShader, Vector2(minimapSize.x / 2, 540.0f - minimapSize.y / 2), minimapSize, 0.0f));// minimap
	// setup text renderers
	debugText.push_back(new TextRenderer(&engine, &textShader, "Time: ", Vector2(1.0f, 1.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "Fps Avg: ", Vector2(1.0f, 17.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "ms: ", Vector2(1.0f, 33.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	if(engine.ended || !characterMapInitialized) {
		Log("Fonts failed to init");
		engine.Delete();
		return 0;
	}
	// run main loop
	engine.onLoop.push_back(Loop);
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
	delete cam;
	delete uiCam;
}