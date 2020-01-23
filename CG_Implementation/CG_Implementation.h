#pragma once

#include <CG_Engine/CG_Engine.h>
#include <CG_Engine/Shader.h>
#include <CG_Engine/Camera.h>
#include <CG_Engine/InputHandler.h>
#include <CG_Engine/Renderer.h>
#include <CG_Engine/Cubemap.h>
#include <CG_Engine/ParticleSystem.h>
#include <CG_Engine/CgTime.h>
#include <CG_Engine/PostProcessing.h>
#include <CG_Engine/ModelLoader.h>
#include <CG_Engine/Terrain.h>
#include <filesystem>

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
	void cleanup();
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
	std::shared_ptr<CG_Data::UBO> CameraUBO;
	std::shared_ptr<CG_Data::UBO> LightUBO;
	CameraUboData CameraUBOData;
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
	std::filesystem::path AssetBase = "./assets/";	//This is the base folder of all program assets
	std::filesystem::path ModelBase = AssetBase / "models/";	//This is the base folder of all models
	std::filesystem::path barrelBase = ModelBase / "barrel/";
	std::filesystem::path barrelModelPath = barrelBase / "barrel.obj";
	std::filesystem::path barrel_diff_name = barrelBase / "textures/barrel.png";
	std::filesystem::path barrel_normal_name = barrelBase / "textures/barrelNormal.png";
	std::filesystem::path kitchen_base = "assets/models/kitchen/";
	std::filesystem::path kitchenModelPath = kitchen_base / "kitchen.obj";
	std::filesystem::path nanosuit_base = ModelBase / "nanosuit/";
	std::filesystem::path nanosuitModelPath = nanosuit_base / "nanosuit.obj";
	std::filesystem::path sun_base = ModelBase / "sun/";
	std::filesystem::path sunModelPath = sun_base / "sun.obj";
	std::filesystem::path waterDUDV_loc = ModelBase / "water/waterDUDV.png";
	std::filesystem::path GrassLoc = ModelBase / "textures/grass.png";
	std::filesystem::path dragon_base = "assets/models/dragon/";
	std::filesystem::path dragonModelPath = dragon_base / "dragon_blender2.dae";

	std::filesystem::path basicVLoc = "v.glsl", basicFLoc = "f.glsl";
	std::filesystem::path sunVLoc = "sunV.glsl", sunFLoc = "sunF.glsl";
	std::filesystem::path skyboxVLoc = "skyboxV.glsl", skyboxFLoc = "skyboxF.glsl";
	std::filesystem::path groundVLoc = "groundV.glsl", groundFLoc = "groundF.glsl";
	std::filesystem::path waterVLoc = "waterV.glsl", waterFLoc = "waterF.glsl";
	std::filesystem::path guiVLoc = "guiV.glsl", guiFLoc = "guiF.glsl";
	std::filesystem::path kitchenVLoc = "kitchenV.glsl", kitchenFLoc = "kitchenF.glsl";
	std::filesystem::path nanosuitVShader = "nanosuitV.glsl", nanosuitFShader = "nanosuitF.glsl";
	std::filesystem::path RiggedDragonVShader = "RiggedDragonV.glsl", RiggedDragonFShader = "RiggedDragonF.glsl";
	std::vector<std::string> SkyboxTexLoc{ "./assets/skybox/right.png", "./assets/skybox/left.png", "./assets/skybox/top.jpg",
		"./assets/skybox/bottom.png", "./assets/skybox/back.png", "./assets/skybox/front.png" };

};
	

