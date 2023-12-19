#pragma once

#ifndef _POINTLIGHT
#define _POINTLIGHT

#include <string>
#include "Entity.h"
#include "FBO.h"

class PointLight : public Entity
{
public:
	PointLight(std::string _name, unsigned int width, unsigned int height);
	~PointLight();
	FBO* getShadowFBO();

private:
	FBO* shadowFBO;

	unsigned int resW, resH;
};

#endif