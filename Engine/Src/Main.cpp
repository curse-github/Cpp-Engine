#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
#include "Engine.h"

float mapScale=35.0f;
float spacing=0.05f;
Vector2 mapSize=Vector2(49.0f, 34.0f);
Vector2 offset=Vector2(26.5f, 30.0f);
const float minimapScale=0.15f;
Vector2 minimapSize;
Vector2 fullMapSize=mapSize * (1 + spacing) * mapScale;

float playerSize=0.5f;
float playerSpeed=4.0f;

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
Vector2 gridToWorld(Vector2 grid) {
	return Vector2(grid.x * mapScale * (1 + spacing), (mapSize.y - grid.y) * mapScale * (1 + spacing));
}
Vector2 worldToGrid(Vector2 world) {
	return Vector2(world.x / mapScale / (1 + spacing), mapSize.y - world.y / mapScale / (1 + spacing));
}
Vector2 gridToMinimap(Vector2 grid) {
	return Vector2(grid.x / mapSize.x * minimapSize.x, 540 - grid.y / mapSize.y * minimapSize.y);
}
class PlayerController : public Object {
protected:
	PlayerController* self;
	SpriteRenderer* playerRenderer;
	SpriteRenderer* playerIconRenderer;
	OrthoCam* sceneCam;
public:
	Vector2 pos;
	PlayerController() : Object(), self(nullptr), playerRenderer(nullptr), playerIconRenderer(nullptr), sceneCam(nullptr), pos(Vector2(0)) {}
	PlayerController(Engine* _engine, SpriteRenderer* _playerRenderer, SpriteRenderer* _playerIconRenderer, OrthoCam* _sceneCam)
		: Object(_engine), self(this), playerRenderer(_playerRenderer), playerIconRenderer(_playerIconRenderer), sceneCam(_sceneCam), pos(Vector2(0)) {
		if(!initialized) return;
		sub_key();
		sub_loop();
		pos=worldToGrid(playerRenderer->position);
		sceneCam->position=playerRenderer->position;
		playerIconRenderer->position=gridToMinimap(pos);
		sceneCam->update();
		sceneCam->use();
	}
	int inputs[4]={ GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE };
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if(engine->ended || !initialized) return;
		if(key == GLFW_KEY_W) inputs[0]=action;
		else if(key == GLFW_KEY_A) inputs[1]=action;
		else if(key == GLFW_KEY_S) inputs[2]=action;
		else if(key == GLFW_KEY_D) inputs[3]=action;
	}
	void on_loop(double delta) {
		if(engine->ended || !initialized) return;
		Vector2 inputVec=Vector2(
			(float)(inputs[3] >= GLFW_PRESS) - (float)(inputs[1] >= GLFW_PRESS),
			(float)(inputs[2] >= GLFW_PRESS) - (float)(inputs[0] >= GLFW_PRESS)
		).normalized() * ((float)delta) * playerSpeed;
		if(inputVec.x == 0 && inputVec.y == 0) return;
		pos+=inputVec;
		Vector2 worldPos=gridToWorld(pos);
		playerRenderer->position=worldPos;
		sceneCam->position=worldPos;
		playerIconRenderer->position=gridToMinimap(pos);
		sceneCam->update();
		sceneCam->use();
	}
};

void Loop(double delta);
void onDelete();

Engine engine;
FpsTracker tracker;
OrthoCam* cam;
OrthoCam* uiCam;

Shader playerShader;
Shader playerIconShader;
SpriteRenderer playerRenderer;
SpriteRenderer playerIconRenderer;

Shader backgroundShader;
vector<SpriteRenderer*> sceneRenderers;
Shader minimapShader;
vector<SpriteRenderer*> uiRenderers;
Shader textShader;
vector<TextRenderer*> debugText;

PlayerController* playerController;

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
	minimapSize=Vector2((float)minimapTex.width, (float)minimapTex.height) * minimapScale;
	// setup shaders
	playerShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	playerIconShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	backgroundShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	minimapShader=Shader(&engine, "Shaders/vs.glsl", "Shaders/texFrag.glsl");
	textShader=Shader(&engine, "Shaders/textVs.glsl", "Shaders/textFrag.glsl");
	if(engine.ended ||
		!playerShader.initialized || !playerIconShader.initialized ||
		!backgroundShader.initialized || !minimapShader.initialized ||
		!textShader.initialized
		) {
		Log("Shaders failed to init");
		engine.Delete();
		return 0;
	}
	// set shader constants
	playerShader.setTexture("_texture", &playerTex, 0);
	playerShader.setFloat4("modulate", Vector4(0.35f, 0.0f, 0.7f, 1.0f));// purple
	playerIconShader.setTexture("_texture", &playerTex, 0);
	playerIconShader.setFloat4("modulate", Vector4(0.35f, 0.0f, 0.7f, 0.75f));// purple
	backgroundShader.setTexture("_texture", &backgroundTex, 0);
	minimapShader.setTexture("_texture", &minimapTex, 0);
	minimapShader.setFloat4("modulate", Vector4(1.0f, 1.0f, 1.0f, 0.75f));// half transparency
	cam->bindShader(&playerShader);
	uiCam->bindShader(&playerIconShader);
	cam->bindShader(&backgroundShader);
	uiCam->bindShader(&minimapShader);
	uiCam->bindShader(&textShader);
	cam->use();
	uiCam->use();
	// player
	playerRenderer=SpriteRenderer(&engine, &playerShader, gridToWorld(offset), playerSize * mapScale, 0.0f);
	playerRenderer.zIndex=1;
	sceneRenderers.push_back(&playerRenderer);
	// playerIcon
	playerIconRenderer=SpriteRenderer(&engine, &playerIconShader, Vector2(minimapSize.x / 2, 540.0f - minimapSize.y / 2), Vector2(minimapSize.x / mapSize.x, minimapSize.y / mapSize.y), 0.0f);
	playerIconRenderer.zIndex=1;
	uiRenderers.push_back(&playerIconRenderer);
	// background
	SpriteRenderer bgRenderer=SpriteRenderer(&engine, &backgroundShader, fullMapSize / 2.0f, fullMapSize, 0.0f);
	sceneRenderers.push_back(&bgRenderer);
	// minimap
	uiRenderers.push_back(new SpriteRenderer(&engine, &minimapShader, Vector2(minimapSize.x / 2, 540.0f - minimapSize.y / 2), minimapSize, 0.0f));
	// setup text renderers
	debugText.push_back(new TextRenderer(&engine, &textShader, "Time: ", Vector2(1.0f, 1.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "Fps Avg: ", Vector2(1.0f, 17.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	debugText.push_back(new TextRenderer(&engine, &textShader, "Pos: ", Vector2(1.0f, 33.0f), 2.0f, Vector3(1.0f, 1.0f, 1.0f)));
	if(engine.ended || !characterMapInitialized) {
		Log("Fonts failed to init");
		engine.Delete();
		return 0;
	}
	// setup other stuff
	playerController=new PlayerController(&engine, &playerRenderer, &playerIconRenderer, cam);
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
	debugText[2]->text="Pos: " + playerController->pos.to_string();
	// draw scene
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
	delete cam; delete uiCam; delete playerController;
}