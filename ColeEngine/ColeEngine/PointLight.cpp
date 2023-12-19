#include "PointLight.h"

PointLight::PointLight(std::string _name, unsigned int width, unsigned int height)
	: shadowFBO(nullptr), resW(width), resH(height)
{

	shadowFBO = new FBO();

	if (shadowFBO)
	{
		shadowFBO->CreateFBO_3D(resW, resH, false);
	}

	setName(_name);
}

PointLight::~PointLight()
{
	if (shadowFBO)
	{
		delete shadowFBO;
		shadowFBO = nullptr;
	}
}

FBO* PointLight::getShadowFBO()
{
	if (shadowFBO)
	{
		return shadowFBO;
	}
}