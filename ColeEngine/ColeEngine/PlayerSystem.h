#pragma once


#ifndef _PLAYERSYSTEM
#define _PLAYERSYSTEM

#include "Engine.h"
#include "System.h"

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/ext.hpp>

class PlayerSystem : public System
{
public:
	explicit PlayerSystem(Engine& _engine);

	void update(Engine& engine) override;

private:

};

#endif