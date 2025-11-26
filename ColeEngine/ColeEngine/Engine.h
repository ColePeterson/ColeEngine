#pragma once

#ifndef _ENGINE
#define _ENGINE

#include "Platform.h"
#include "World.h"
#include "ResourceManager.h"

#include "System.h"


//#include "Logging.h"

class World;

enum class EngineMode
{
	EDITOR,
	TERRAIN
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
	Entity* selectedEntityPrevious;
	Entity* playerEntity;

	bool onSelect;

	EngineMode mode;

private:

	Platform& platform;
	World& world;
	ResourceManager& resource;

};

#endif