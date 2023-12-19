#pragma once

#ifndef _UI
#define _UI


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Engine.h"


struct SceneBrowser
{
	int selected = -1;
	Entity* selectedEntity = nullptr;
};



struct SceneLighting
{
	float ambientColor[4] = { 0.1f, 0.12f, 0.12f, 1.0f };
};


struct ComponentTab
{
	Material* selectedMat = nullptr;
};

class UI
{
public:
	UI(Engine& _engine);

	void draw();
	void sceneLighting_draw();
	void sceneBrowser_draw();
	void componentTab_draw();

	void transformComp_draw();
	void renderComp_draw();
	void pointLightComp_draw();

private:
	Engine& engine;

	std::string selectedTextureName = "Texture";
	std::string selectedMaterialName = "Material";
	SceneBrowser sceneBrowser;
	SceneLighting sceneLighting;
	//ComponentTab componentTab;
	
};

#endif