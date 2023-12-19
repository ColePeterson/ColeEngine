#pragma once

#ifndef _ENGINE
#define _ENGINE

#include "Platform.h"
#include "World.h"
#include "ResourceManager.h"
//#include "Logging.h"

enum class EngineMode
{
	EDITOR,
	PLAY
};


struct DebugValues
{
	float float1 = 0.0f;
	float float2 = 0.0f;
	bool drawDebugLines = false;
	bool basicShading = false;
};

class Engine
{
public:
	Engine(Platform& _platform, World& _world, ResourceManager& _resource);

	Platform& getPlatform() { return platform; }

	World& getWorld() { return world; }

	ResourceManager& Resource() { return resource; }

	

	DebugValues debug;

	Entity* selectedEntity;
	bool onSelect;

private:

	Platform& platform;
	World& world;
	ResourceManager& resource;

	

	EngineMode mode;
};

#endif