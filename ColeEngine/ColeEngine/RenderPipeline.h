#pragma once

#ifndef _RENDERPIPELINE
#define _RENDERPIPELINE

#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Entity.h"
#include "Engine.h"
#include "Material.h"
#include "GBuffer.h"

struct GeometryPassUniforms
{
	glm::mat4 worldProj = glm::mat4();
	glm::mat4 worldView = glm::mat4();
	glm::mat4 worldInverse = glm::mat4();
	glm::mat4 transform = glm::mat4();

};

struct LightingPassUniforms
{
	std::vector<glm::vec3> pointLightPos;
};


struct ShadowPassUniforms
{
	glm::mat4 lightProj = glm::mat4();
	glm::mat4 lightView = glm::mat4();
	glm::mat4 transform = glm::mat4();
};

struct PointLightShadowPassUniforms
{
	glm::mat4 shadowProj;
	std::vector<glm::mat4> shadowTransforms;

	float farPlane = 25.0f;
	glm::vec3 pointLightPos = { 6.0f, 0.0f, 2.0f };
	unsigned int textureID;
};

class RenderPipeline
{
public:
	RenderPipeline(std::string _name, ShaderProgram* _shader);

	void draw(RenderComponent* render);

	void setGeometryPassUnis(Engine& engine, GeometryPassUniforms uniforms);
	void setLightingPassUnis(Engine& engine, GBuffer* gBuffer, LightingPassUniforms uniforms);

	//void setLightingPassUnis(Engine& engine, RenderComponent* render, LightingPassUniforms uniforms);
	void setShadowPassUnis(Engine& engine, RenderComponent* render, ShadowPassUniforms uniforms);
	void setPointShadowPassUnis(Engine& engine, PointLightShadowPassUniforms pointPassUnis);

	//void setMaterialUniforms(Engine& engine, LightingPassUniforms lightingUnis, PointLightShadowPassUniforms pointunis, Material* mat);
	void setMaterialUniforms(Engine& engine, Material* mat);
	std::string getName() { return name; }

	ShaderProgram* shader = nullptr;

private:
	std::string name;
};

#endif