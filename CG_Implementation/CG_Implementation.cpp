#include "CG_Implementation.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <thread>

float guiVertices[]{
	-1, 1, -1.0,
	-1, -1, -1.0,
	1, -1, -1.0,
	1, 1, -1.0
};

float guiTexCoords[]{
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0
};

unsigned int guiIndices[]{
	0, 1, 3,
	1, 2, 3
};


float waterPlaneVert[]{
	-1, 0, 1,
	-1, 0, -1,
	1, 0, -1,
	1, 0, 1,
};
unsigned int waterPlaneIndices[]{
	0, 1, 3,
	1, 2, 3
};

using namespace GL_Engine;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//Callbacks for key events
static uint8_t activeEnt = 0;
long long time_millis_camera = 0;
void CameraKeyEvent(GLuint Key, void* Parameter) {
	Camera *camera = static_cast<Camera*>(Parameter);
	auto speed = 20.0f;// metres per second
	auto time_diff_sec = time_millis_camera / (float) 1e6;
	auto amount = speed * time_diff_sec;
	float dX = Key == GLFW_KEY_A ? -amount : Key == GLFW_KEY_D ? amount : 0;
	float dY = Key == GLFW_KEY_LEFT_SHIFT ? amount : Key == GLFW_KEY_LEFT_CONTROL ? -amount : 0;
	float dZ = Key == GLFW_KEY_W ? amount : Key == GLFW_KEY_S ? -amount : 0;
	camera->translateCamera(glm::vec4(dX, dY, dZ, 1.0));

	amount *= 4;
	float yaw = Key == GLFW_KEY_Q ? amount : Key == GLFW_KEY_E ? -amount : 0;
	camera->yawBy(yaw);

	//if (Key == GLFW_KEY_Q)
	//	camera->ReflectCamera();

	float pitch = Key == GLFW_KEY_Z ? amount : Key == GLFW_KEY_X ? -amount : 0;
	camera->pitchBy(pitch);
}

void CubeKeyEvent(GLuint Key, void* Parameter) {
	LightUBO_Data *data = (LightUBO_Data*)Parameter;
	auto speed = 20.0f;// metres per second
	auto time_diff_sec = time_millis_camera / (float) 1e6;
	auto amount = speed * time_diff_sec;

	switch (Key) {
	
	case GLFW_KEY_LEFT :
		data->LightPosition[0] += amount;
		break;
	case GLFW_KEY_RIGHT:
		data->LightPosition[0] -= amount;
		break;
	case GLFW_KEY_UP:
		data->LightPosition[1] += amount;
		break;
	case GLFW_KEY_DOWN:
		data->LightPosition[1] -= amount;
		break;
	case GLFW_KEY_INSERT:
		data->LightPosition[2] += amount;
		break;
	case GLFW_KEY_DELETE:
		data->LightPosition[2] -= amount;
		break;
	}
}

bool FlameOn{ false };
void DragonKeyEvent(GLuint Key, void* Parameter) {
	FlameOn = !FlameOn;
}

static bool wireframe = false;
void WireframeEvent(GLuint Key, void *Parameter) {
	wireframe = !wireframe;
}

CG_Implementation::CG_Implementation(){
}


int CG_Implementation::run(){
	initialise();

	while (!glfwWindowShouldClose(windowProperties.window)){
		//Get the framerate and show it as the window's title
		uint64_t time_diff = FramerateStopwatch.MeasureTime().count();
		double second_diff = time_diff / 1.0e6;
		double fps = 1.0 / second_diff;
		char title[50];
		snprintf(title, 50, "FPS: %f", fps);
		glfwSetWindowTitle(windowProperties.window, title);
		
		//Lap the camera stopwatch for the camera key callback
		time_millis_camera = CameraStopwatch.MeasureTime().count();
		//Check for key events
		keyHandler.Update(windowProperties.window);

		//Update the matrices of all the entities
		barrel.GetTransformMatrix();
		kitchen.GetTransformMatrix();
		nanosuit.GetTransformMatrix();
		sun.GetTransformMatrix();

		time += (float)second_diff;
		DragonRiggedModel->Update(1, time);	//Update the dragon's animation
		//Make the dragon fly around in a circle
		float circleRadius = 250;
		float xPos = circleRadius * cos(time);
		float zPos = circleRadius * sin(time);
		DragonRiggedModel->SetPosition(glm::vec3(xPos, 25, zPos));
		DragonRiggedModel->YawBy(glm::degrees((float)second_diff));
		//Move the particle system to the mouth of the dragon
		particleTransform = DragonRiggedModel->GetTransformMatrix() * particleSystem->GetTransformMatrix();
		particleSystem->UpdateTime((float)second_diff);
		particleSystem->SetActive(FlameOn);

		
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		//Water reflection render pass
		glEnable(GL_CLIP_DISTANCE0);
		CameraUBOData.clippingPlane[3] = 0;
		CameraUBOData.clippingPlane[1] = 1;
		camera.reflectCamera();
		UpdateCameraUBO();
		water.Deactivate();
		WaterFBO->bind(0);
		renderer->Render();

		//Water refraction render pass
		CameraUBOData.clippingPlane[1] = -1;
		camera.reflectCamera();
		UpdateCameraUBO();
		water.Deactivate();
		WaterFBO->bind(1);
		renderer->Render();
		WaterFBO->unbind();

		//Main render pass to post processing FBO
		const GLenum CAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		ppFBO->bind(2, CAttachments);

		glDisable(GL_CLIP_DISTANCE0);
		CameraUBOData.clippingPlane[3] = 1000;
		water.Activate();
		renderer->Render();
		ppFBO->unbind();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//Process the FBO
		postprocessPipeline.Process();

		//Output final image to screen
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		guiRenderer->Render();

	
		//Swap buffers
		glfwSwapBuffers(windowProperties.window);
		glfwPollEvents();
		
	}
	return 0;
}

void CG_Implementation::UpdateCameraUBO() {
	memcpy(CameraUBOData.viewMatrix, glm::value_ptr(camera.getViewMatrix()), sizeof(float) * 16);
	memcpy(CameraUBOData.projectionMatrix, glm::value_ptr(camera.getProjectionMatrix()), sizeof(float) * 16);
	glm::mat4 PV = camera.getProjectionMatrix() * camera.getViewMatrix();
	memcpy(CameraUBOData.pvMatrix, glm::value_ptr(PV), sizeof(float) * 16);
	memcpy(CameraUBOData.cameraOrientation, glm::value_ptr(glm::vec4(camera.getForwardVector(), 0.0)), sizeof(float) * 4);
	memcpy(CameraUBOData.cameraPosition, glm::value_ptr(camera.getCameraPosition()), sizeof(float) * 4);
	
	LightUBOData.LightBrightness = 1;
	sun.SetPosition(glm::vec3(LightUBOData.LightPosition[0], LightUBOData.LightPosition[1], LightUBOData.LightPosition[2]));
}

void CG_Implementation::initialise(){
	
	//Initialise window and Glad
	if (!engine.CG_CreateWindow(&windowProperties)){
		throw std::runtime_error("Error initialising GLFW!");
	}
	
	Properties::GLADproperties gladProps;
	if (!engine.CG_StartGlad(&gladProps)){
		throw std::runtime_error("Error initialising GLAD!");
	}

	//Initialise camera
	camera.setCameraPosition(glm::vec4(0, 10, -3, 1.0));		//Set initial camera position
	camera.setProjectionMatrix(0.01f, 10000.0f, 75.0f, 
		(float)windowProperties.width / (float)windowProperties.height);	//Set the Projection Matrix

	//Set the light's initial position
	memcpy(LightUBOData.LightPosition, glm::value_ptr(glm::vec4(0, 20, -20, 1.0)), sizeof(float) * 4);
	//Set the clipping plane for the water
	CameraUBOData.clippingPlane[0] = 0;
	CameraUBOData.clippingPlane[1] = 1;
	CameraUBOData.clippingPlane[2] = 0;
	CameraUBOData.clippingPlane[3] = 0;

	SetupShaders();
	LoadModels();
	SetupModels();
	SetupKeyEvents();
	glEnable(GL_DEPTH_TEST);

	//Setup the camera stopwatch, used for timing the motion of the camera
	CameraStopwatch.Initialise();
	
}

void CG_Implementation::SetupShaders() {
	CameraUBO = std::make_shared<CG_Data::UBO>((void*)&CameraUBOData, sizeof(CameraUBOData));
	LightUBO = std::make_shared<CG_Data::UBO>((void*)&LightUBOData, sizeof(LightUBOData));
	memcpy(LightUBOData.LightColour, glm::value_ptr(glm::vec3(1, 1, 1)), sizeof(float) * 3);

	renderer = std::make_unique<Renderer>();
	DragonRenderer = std::make_unique<Renderer>();
	guiRenderer = std::make_unique<Renderer>();
	renderer->AddUBO(CameraUBO.get());
	renderer->AddUBO(LightUBO.get());

	basicShader.registerShaderStageFromFile(basicVLoc.c_str(), GL_VERTEX_SHADER);
	basicShader.registerShaderStageFromFile(basicFLoc.c_str(), GL_FRAGMENT_SHADER);
	basicShader.registerAttribute("vPosition", 0);
	basicShader.registerAttribute("vNormal", 2);
	basicShader.registerAttribute("TexCoord", 1);
	basicShader.registerAttribute("vTangeant", 3);
	basicShader.registerAttribute("vBitangeant", 4);
	basicShader.registerTextureUnit("diffuseTexture", 0);
	basicShader.registerTextureUnit("normalTexture", 1);
	basicShader.registerUBO(std::string("CameraProjectionData"), CameraUBO );
	basicShader.registerUBO(std::string("LightData"), LightUBO );
	basicShader.registerUniform("model");
	basicShader.compileShader();

	groundShader.registerShaderStageFromFile(groundVLoc.c_str(), GL_VERTEX_SHADER);
	groundShader.registerShaderStageFromFile(groundFLoc.c_str(), GL_FRAGMENT_SHADER);
	groundShader.registerAttribute("MeshXZ", 0);
	groundShader.registerAttribute("Height", 1);
	groundShader.registerAttribute("TexCoords", 2);
	groundShader.registerAttribute("Normals", 3);
	groundShader.registerTextureUnit("GrassTexture", 0);
	groundShader.registerUBO(std::string("CameraProjectionData"), CameraUBO );
	groundShader.registerUBO(std::string("LightData"), LightUBO );
	groundShader.registerUniform("GroundTranslation");
	groundShader.compileShader();

	waterShader.registerShaderStageFromFile(waterVLoc.c_str(), GL_VERTEX_SHADER);
	waterShader.registerShaderStageFromFile(waterFLoc.c_str(), GL_FRAGMENT_SHADER);
	waterShader.registerAttribute("vPosition", 0);
	waterShader.registerTextureUnit("reflectionTexture", 0);
	waterShader.registerTextureUnit("refractionTexture", 1);
	waterShader.registerTextureUnit("dudvMap", 2);
	waterShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	waterShader.registerUniform("Time");
	waterShader.compileShader();

	nanosuitShader.registerShaderStageFromFile(nanosuitVShader.c_str(), GL_VERTEX_SHADER);
	nanosuitShader.registerShaderStageFromFile(nanosuitFShader.c_str(), GL_FRAGMENT_SHADER);
	nanosuitShader.registerAttribute("vPosition", 0);
	nanosuitShader.registerAttribute("vNormal", 2);
	nanosuitShader.registerAttribute("TexCoord", 1);
	nanosuitShader.registerAttribute("vTangeant", 3);
	nanosuitShader.registerAttribute("vBitangeant", 4);
	nanosuitShader.registerTextureUnit("diffuseTexture", 0);
	nanosuitShader.registerTextureUnit("normalTexture", 1);
	nanosuitShader.registerTextureUnit("specularTexture", 2);
	nanosuitShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	nanosuitShader.registerUBO(std::string("LightData"), LightUBO);
	nanosuitShader.registerUniform("model");
	nanosuitShader.compileShader();

	RiggedDragonShader.registerShaderStageFromFile(RiggedDragonVShader.c_str(), GL_VERTEX_SHADER);
	RiggedDragonShader.registerShaderStageFromFile(RiggedDragonFShader.c_str(), GL_FRAGMENT_SHADER);
	RiggedDragonShader.registerAttribute("vPosition", 0);
	RiggedDragonShader.registerAttribute("vNormal", 2);
	RiggedDragonShader.registerAttribute("TexCoord", 1);
	RiggedDragonShader.registerAttribute("vTangeant", 3);
	RiggedDragonShader.registerAttribute("vBitangeant", 4);
	RiggedDragonShader.registerAttribute("BoneIDs", 5);
	RiggedDragonShader.registerAttribute("BoneWeights", 6);
	RiggedDragonShader.registerTextureUnit("diffuseTexture", 0);
	RiggedDragonShader.registerTextureUnit("normalTexture", 1);
	RiggedDragonShader.registerTextureUnit("specularTexture", 2);
	RiggedDragonShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	RiggedDragonShader.registerUBO(std::string("LightData"), LightUBO);
	RiggedDragonShader.registerUniform("model");
	RiggedDragonShader.registerUniform("BoneMatrices");
	RiggedDragonShader.compileShader();


	kitchenShader.registerShaderStageFromFile(kitchenVLoc.c_str(), GL_VERTEX_SHADER);
	kitchenShader.registerShaderStageFromFile(kitchenFLoc.c_str(), GL_FRAGMENT_SHADER);
	kitchenShader.registerAttribute("vPosition", 0);
	kitchenShader.registerAttribute("TexCoord", 1);
	kitchenShader.registerAttribute("vNormal", 2);
	kitchenShader.registerTextureUnit("diffuseTexture", 0);
	kitchenShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	kitchenShader.registerUBO(std::string("LightData"), LightUBO);
	kitchenShader.registerUniform("model");
	kitchenShader.compileShader();

	sunShader.registerShaderStageFromFile(sunVLoc.c_str(), GL_VERTEX_SHADER);
	sunShader.registerShaderStageFromFile(sunFLoc.c_str(), GL_FRAGMENT_SHADER);
	sunShader.registerAttribute("vPosition", 0);
	sunShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	sunShader.registerUniform("model");
	sunShader.compileShader();

	SkyboxShader.registerShaderStageFromFile(skyboxVLoc.c_str(), GL_VERTEX_SHADER);
	SkyboxShader.registerShaderStageFromFile(skyboxFLoc.c_str(), GL_FRAGMENT_SHADER);
	SkyboxShader.registerAttribute("vPosition", 1);
	SkyboxShader.registerTextureUnit("BoxTexture", 0);
	SkyboxShader.registerUBO(std::string("CameraProjectionData"), CameraUBO);
	SkyboxShader.compileShader();
	int loc = glGetAttribLocation(SkyboxShader.getShaderID(), "vPosition");
	Skybox = std::make_unique<Cubemap>(SkyboxTexLoc, &SkyboxShader, renderer.get());

	guiShader.registerShaderStageFromFile(guiVLoc.c_str(), GL_VERTEX_SHADER);
	guiShader.registerShaderStageFromFile(guiFLoc.c_str(), GL_FRAGMENT_SHADER);
	guiShader.registerAttribute("vPosition", 0);
	guiShader.registerAttribute("TexCoord", 1);
	guiShader.registerTextureUnit("image", 0);
	guiShader.registerTextureUnit("brightness", 1);
	guiShader.compileShader();

}

void CG_Implementation::SetupModels() {
	//Set the update callbacks for the various uniforms using Lambda functions
	auto FloatLambda = [](const CG_Data::Uniform &u) {glUniform1fv(u.GetID(), 1, static_cast<const GLfloat*>(u.GetData())); };
	auto MatrixLambda = [](const CG_Data::Uniform &u) {glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, static_cast<const GLfloat*>(u.GetData())); };

	const auto width = windowProperties.width, height = windowProperties.height;

	auto BasicModelUniform = basicShader.getUniform("model");
	BasicModelUniform->SetUpdateCallback(MatrixLambda);
	auto KitchenModelUniform = kitchenShader.getUniform("model");
	KitchenModelUniform->SetUpdateCallback(MatrixLambda);
	auto NanosuitModelUniform = nanosuitShader.getUniform("model");
	NanosuitModelUniform->SetUpdateCallback(MatrixLambda);
	auto WaterTimeUniform = waterShader.getUniform("Time");
	WaterTimeUniform->SetUpdateCallback(FloatLambda);

	GLsizei bCount;
	auto nodeModelIndex = barrel.AddData((void*)glm::value_ptr(barrel.TransformMatrix));
	barrel.SetPosition(glm::vec3(0, 0, 0));
	bCount = (GLsizei)barrelAttributes[0]->GetVertexCount();
	auto barrelPass = renderer->AddRenderPass(&basicShader);
	barrelPass->SetDrawFunction([bCount]() {glDrawElements(GL_TRIANGLES, bCount, GL_UNSIGNED_INT, 0); });
	barrelPass->BatchVao = barrelAttributes[0];
	std::move(barrelAttributes[0]->ModelTextures.begin(), barrelAttributes[0]->ModelTextures.end(), std::back_inserter(barrelPass->Textures));
	barrelPass->AddBatchUnit(&barrel);
	barrelPass->AddDataLink(BasicModelUniform.get(), nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	barrel.Translate(glm::vec3(3, 4, 8));

	nodeModelIndex = kitchen.AddData((void*)glm::value_ptr(kitchen.TransformMatrix));
	kitchen.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : kitchenAttributes) {
		GLsizei kCount = (GLsizei)a->GetVertexCount();
		auto kitchenPass = renderer->AddRenderPass(&kitchenShader);
		kitchenPass->SetDrawFunction([kCount]() {glDrawElements(GL_TRIANGLES, kCount, GL_UNSIGNED_INT, 0); });
		kitchenPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(kitchenPass->Textures));
		kitchenPass->AddBatchUnit(&kitchen);
		kitchenPass->AddDataLink(KitchenModelUniform.get(), nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}
	kitchen.ScaleBy(glm::vec3(0.3, 0.3, 0.3));
	kitchen.Translate(glm::vec3(0, -2, 0));

	nodeModelIndex = nanosuit.AddData((void*)glm::value_ptr(nanosuit.TransformMatrix));
	nanosuit.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : nanosuitAttributes) {
		GLsizei nCount = (GLsizei)a->GetVertexCount();
		auto nanosuitPass = renderer->AddRenderPass(&nanosuitShader);
		nanosuitPass->SetDrawFunction([nCount]() {glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_INT, 0); });
		nanosuitPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(nanosuitPass->Textures));
		nanosuitPass->AddBatchUnit(&nanosuit);
		nanosuitPass->AddDataLink(NanosuitModelUniform.get(), nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}


	renderer->AddRenderPass(std::move(DragonRiggedModel->GenerateRenderpass(&RiggedDragonShader)));
	DragonRiggedModel->SetPosition(glm::vec3(0, 25, 0));
	DragonRiggedModel->PitchBy(90.0f);
	DragonRiggedModel->ScaleBy(glm::vec3(10, 10, 10));

	nodeModelIndex = sun.AddData((void*)glm::value_ptr(sun.TransformMatrix));
	sun.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : sunAttributes) {
		GLsizei nCount = (GLsizei)a->GetVertexCount();
		auto sunPass = renderer->AddRenderPass(&sunShader);
		sunPass->SetDrawFunction([nCount]() {glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_INT, 0); });
		sunPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(sunPass->Textures));
		sunPass->AddBatchUnit(&sun);
		sunPass->AddDataLink(KitchenModelUniform.get(), nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}



	WaterFBO = std::make_unique<CG_Data::FBO>(CG_Engine::ViewportWidth, CG_Engine::ViewportHeight);
	auto reflectColAttach = WaterFBO->addAttachment(CG_Data::FBO::AttachmentType::ColourTexture, windowProperties.width, windowProperties.height);
	auto refractColAttach = WaterFBO->addAttachment(CG_Data::FBO::AttachmentType::ColourTexture, windowProperties.width, windowProperties.height);
	WaterFBO->addAttachment(CG_Data::FBO::AttachmentType::DepthTexture, windowProperties.width, windowProperties.height);
	auto reflectionTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(reflectColAttach)->GetTexture();
	auto refractionTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(refractColAttach)->GetTexture();
	reflectionTexture->SetUnit(GL_TEXTURE0);
	refractionTexture->SetUnit(GL_TEXTURE1);

	waterDUDVTexture = ModelLoader::loadTexture(waterDUDV_loc, GL_TEXTURE2);
	waterVAO = std::make_shared<CG_Data::VAO>();
	waterVAO->BindVAO();
	auto vertexVBO = std::make_unique<CG_Data::VBO>(&waterPlaneVert[0], 12 * sizeof(float), GL_STATIC_DRAW);
	auto indices = std::make_unique<CG_Data::VBO>(&waterPlaneIndices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	vertexVBO->BindVBO();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	waterVAO->AddVBO(std::move(vertexVBO));
	waterVAO->AddVBO(std::move(indices));
	auto waterTimeIndex = water.AddData(&time);
	auto waterRenderPass = renderer->AddRenderPass(&waterShader);
	waterRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); });
	waterRenderPass->Textures.push_back(reflectionTexture);
	waterRenderPass->Textures.push_back(refractionTexture);
	waterRenderPass->Textures.push_back(waterDUDVTexture);
	waterRenderPass->BatchVao = waterVAO;
	waterRenderPass->AddDataLink(WaterTimeUniform.get(), waterTimeIndex);
	waterRenderPass->AddBatchUnit(&water);

	GrassTexture = ModelLoader::loadTexture(GrassLoc, GL_TEXTURE0);

	terrain = std::make_unique<Terrain>(256, 256);
	terrain->GenerateChunk(0, 0);
	terrain->GenerateChunk(1, 0);
	terrain->GenerateChunk(1, 1);
	terrain->GenerateChunk(-1, 0);
	terrain->GenerateChunk(-1, -1);
	terrain->GenerateChunk(0, -1);

	auto tRenderPass = terrain->GetRenderPass(&groundShader);
	tRenderPass->Textures.push_back(GrassTexture);
	renderer->AddRenderPass(std::move(tRenderPass));

	ParticleSystem::ParticleStats pStats;
	pStats.BaseDirection = glm::vec3(0.0, 20.0, 80.0);
	pStats.Position = glm::vec3(0.2f, -6.4f, 4.3f);
	pStats.ParticleCount = 500000;
	pStats.ColourRange[0] = glm::vec3(1.0f, 0.4498f, 0.17f);
	pStats.ColourRange[1] = glm::vec3(1.0f, 0.6498f, 0.37);

	particleSystem = std::make_unique<ParticleSystem>();
	auto pRenderer = particleSystem->GenerateParticleSystem(pStats, CameraUBO);
	particleSystem->SetData(0, static_cast<void*>(glm::value_ptr(particleTransform)));
	renderer->AddRenderPass(std::move(pRenderer));
	particleSystem->PitchBy(-90.0f);

	ppFBO = std::make_unique<CG_Data::FBO>(CG_Engine::ViewportWidth, CG_Engine::ViewportHeight);
	auto FragColAttach = ppFBO->addAttachment(CG_Data::FBO::AttachmentType::ColourTexture, windowProperties.width, windowProperties.height);
	auto BrightColAttach = ppFBO->addAttachment(CG_Data::FBO::AttachmentType::ColourTexture, windowProperties.width, windowProperties.height);
	ppFBO->addAttachment(CG_Data::FBO::AttachmentType::DepthTexture, windowProperties.width, windowProperties.height);
	auto FragTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(FragColAttach)->GetTexture();
	auto BrightTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(BrightColAttach)->GetTexture();
	BrightTexture->SetUnit(GL_TEXTURE0);

	CG_Data::Uniform *uni = postprocessPipeline.AddAttachment(PostProcessing::GaussianBlur);
	auto Tex = postprocessPipeline.Compile(BrightTexture, windowProperties.width, windowProperties.height);
	Tex->SetUnit(GL_TEXTURE1);

	guiVAO = std::make_shared<CG_Data::VAO>();
	guiVAO->BindVAO();
	vertexVBO = std::make_unique<CG_Data::VBO>(&guiVertices[0], 12 * sizeof(float), GL_STATIC_DRAW);
	auto texcoordVBO = std::make_unique<CG_Data::VBO>(&guiTexCoords[0], 8 * sizeof(float), GL_STATIC_DRAW);
	indices = std::make_unique<CG_Data::VBO>(&guiIndices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	vertexVBO->BindVBO();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	texcoordVBO->BindVBO();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	guiVAO->AddVBO(std::move(vertexVBO));
	guiVAO->AddVBO(std::move(texcoordVBO));
	guiVAO->AddVBO(std::move(indices));
	auto guiRenderPass = guiRenderer->AddRenderPass(&guiShader);
	guiRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); });
	guiRenderPass->Textures.push_back(FragTexture);
	guiRenderPass->Textures.push_back(Tex);
	guiRenderPass->BatchVao = guiVAO;
	guiRenderPass->AddBatchUnit(&gui);
}

void CG_Implementation::LoadModels() {

	barrelAttributes = mLoader.loadModel(barrelModelPath, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	barrelAttributes[0]->AddTexture(mLoader.loadTexture(barrel_diff_name, GL_TEXTURE0));
	barrelAttributes[0]->AddTexture(mLoader.loadTexture(barrel_normal_name, GL_TEXTURE1));




	kitchenAttributes = mLoader.loadModel(kitchenModelPath, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);



	nanosuitAttributes = mLoader.loadModel(nanosuitModelPath, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	sunAttributes = mLoader.loadModel(sunModelPath, aiProcess_Triangulate |
												aiProcess_JoinIdenticalVertices);

	DragonRiggedModel = mLoader.loadRiggedModel(dragonModelPath, aiProcess_CalcTangentSpace |
															 aiProcess_Triangulate |
															 aiProcess_JoinIdenticalVertices |
															 aiProcess_SortByPType |
															 aiProcess_GenSmoothNormals);
		mLoader.cleanup();

}


void CG_Implementation::SetupKeyEvents() {
	//Register key events
	keyHandler.AddKeyEvent(GLFW_KEY_W, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_A, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_S, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_D, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT_SHIFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT_CONTROL, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_Q, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_E, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_Z, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_X, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_UP, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_DOWN, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_RIGHT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_INSERT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_DELETE, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&LightUBOData);
	keyHandler.AddKeyEvent(GLFW_KEY_F1, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &WireframeEvent, (void*)nullptr);
	keyHandler.AddKeyEvent(GLFW_KEY_T, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &DragonKeyEvent, (void*)nullptr);

}


//Need to clean up gl objects before terminating GLFW
void CG_Implementation::cleanup() {
	postprocessPipeline.Cleanup();
	basicShader.cleanup();
	SkyboxShader.cleanup();
	kitchenShader.cleanup();
	nanosuitShader.cleanup();
	guiShader.cleanup();
	waterShader.cleanup();
	sunShader.cleanup();
	RiggedDragonShader.cleanup();
	groundShader.cleanup();
	guiVAO.reset();
	waterVAO.reset();
	mLoader.cleanup();
	WaterFBO.reset();
	ppFBO.reset();
	renderer.reset();
	guiRenderer.reset();
	DragonRenderer.reset();
	DragonRiggedModel.reset();
	barrelAttributes.clear();
	kitchenAttributes.clear();
	nanosuitAttributes.clear();
	sunAttributes.clear();
	terrain.reset();
	Skybox.reset();
	particleSystem.reset();
	waterDUDVTexture.reset();
	GrassTexture.reset();
	CameraUBO.reset();
	LightUBO.reset();
}

//Cleanup
CG_Implementation::~CG_Implementation(){
	cleanup();
	glfwTerminate();
}


