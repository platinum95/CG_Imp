#pragma once

#include "CG_Engine.h"
#include "Shader.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "Cubemap.h"
#include "ParticleSystem.h"
#include "Time.h"
#include "PostProcessing.h"
#include "ModelLoader.h"
#include "Terrain.h"

static GL_Engine::Properties::GLFWproperties windowProperties = {
	1280,			//Width
	720,			//Height
	"Test Window",	//Title
	nullptr,
	nullptr,
	nullptr,
	false			//Fullscreen
};


struct LightUBO_Data {
	float LightPosition[4];
	float LightColour[3];
	float LightBrightness;
};

using namespace GL_Engine;
class CG_Implementation
{
public:
	CG_Implementation();
	~CG_Implementation();
	void Cleanup();
	int run();

private:
	//The "apps" member functions
	void initialise();
	void SetupShaders();
	void LoadModels();
	void SetupModels();
	void SetupKeyEvents();
	void UpdateCameraUBO();

	//The engine object, doesn't really do anything
	CG_Engine engine;

	//Camera object, takes care of all things camera
	Camera camera;

	//The project's UBOs (lighting and camera) shared by most shaders
	std::unique_ptr<CG_Data::UBO> CameraUBO;
	std::unique_ptr<CG_Data::UBO> LightUBO;
	CameraUBO_Data CameraUBOData;
	LightUBO_Data LightUBOData;

	//Shaders and renderers used in the program
	Shader basicShader, SkyboxShader, kitchenShader, nanosuitShader, guiShader, waterShader, RiggedDragonShader, groundShader, sunShader;
	std::unique_ptr<Renderer> renderer, guiRenderer, DragonRenderer;
	
	//Model stuff, such as the model loader, model data lists, and entity objects
	ModelLoader mLoader;
	ModelAttribList barrelAttributes, kitchenAttributes, nanosuitAttributes, sunAttributes;
	Entity barrel, kitchen, nanosuit, gui, water, sun;
	std::unique_ptr<RiggedModel> DragonRiggedModel;
	std::unique_ptr<Terrain> terrain;
	std::unique_ptr<Cubemap> Skybox;
	std::shared_ptr<CG_Data::Texture> waterDUDVTexture;
	std::shared_ptr<CG_Data::Texture> GrassTexture;
	std::unique_ptr<ParticleSystem> particleSystem;
	glm::mat4 particleTransform;

	//Pipeline for post-processing, in this case for bloom blur
	PostProcessing postprocessPipeline;

	//Single VAOs used by the program
	std::shared_ptr<CG_Data::VAO> guiVAO, waterVAO;

	//The different FBOs used
	std::unique_ptr<CG_Data::FBO> WaterFBO, ppFBO;

	//Stopewatches used for timing
	Stopwatch<std::chrono::microseconds> CameraStopwatch, FramerateStopwatch;

	//Keyhandler object for dealing with keyboard input
	KeyHandler keyHandler;
	
	//Variable used to keep the curent time (in seconds) from render-loop start
	float time{ 0 };
	
	/*String constants*/
	std::string AssetBase = "./assets/";	//This is the base folder of all program assets
	std::string ModelBase = AssetBase + "models/";	//This is the base folder of all models
	std::string barrel_base = ModelBase + "barrel/";
	std::string barrel_model = "barrel.obj";
	std::string barrel_diff_name = barrel_base + "textures/barrel.png";
	std::string barrel_normal_name = barrel_base + "textures/barrelNormal.png";
	std::string kitchen_base = "assets/models/kitchen/";
	std::string kitchen_model = "kitchen.obj";
	std::string nanosuit_base = ModelBase + "nanosuit/";
	std::string nanosuit_model = "nanosuit.obj";
	std::string sun_base = ModelBase + "sun/";
	std::string sun_model = "sun.obj";
	std::string waterDUDV_loc = ModelBase + "water/waterDUDV.png";
	std::string GrassLoc = ModelBase + "textures/grass.png";
	std::string dragon_base = "assets/models/dragon/";
	std::string dragon_model = "dragon_blender2.dae";

	std::string basicVLoc = "v.glsl", basicFLoc = "f.glsl";
	std::string sunVLoc = "sunV.glsl", sunFLoc = "sunF.glsl";
	std::string skyboxVLoc = "skyboxV.glsl", skyboxFLoc = "skyboxF.glsl";
	std::string groundVLoc = "groundV.glsl", groundFLoc = "groundF.glsl";
	std::string waterVLoc = "waterV.glsl", waterFLoc = "waterF.glsl";
	std::string guiVLoc = "guiV.glsl", guiFLoc = "guiF.glsl";
	std::string kitchenVLoc = "kitchenV.glsl", kitchenFLoc = "kitchenF.glsl";
	std::string nanosuitVShader = "nanosuitV.glsl", nanosuitFShader = "nanosuitF.glsl";
	std::string RiggedDragonVShader = "RiggedDragonV.glsl", RiggedDragonFShader = "RiggedDragonF.glsl";
	std::vector<std::string> SkyboxTexLoc{ "./assets/skybox/right.png", "./assets/skybox/left.png", "./assets/skybox/top.jpg",
		"./assets/skybox/bottom.png", "./assets/skybox/back.png", "./assets/skybox/front.png" };

};
	

