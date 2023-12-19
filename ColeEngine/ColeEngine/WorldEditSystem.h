#pragma once


#ifndef _WORLDEDITSYSTEM
#define _WORLDEDITSYSTEM

#include "Engine.h"
#include "System.h"

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/ext.hpp>

class WorldEditSystem : public System
{
public:
	explicit WorldEditSystem(Engine& _engine);

	void update(Engine& engine) override;

private:

	void mouseSelectEntity(Engine& engine);

};

#endif