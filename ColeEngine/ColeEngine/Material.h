#pragma once

#ifndef _MATERIAL
#define _MATERIAL

#define MAX_TEXTURES 16


#include <vector>
#include <string>
#include <unordered_map>

#include "Texture.h"
#include "Entity.h"
#include "Shader.h"

class Entity;


using Color3 = glm::vec3;





struct FloatParam
{
	float val;
	float min;
	float max;

	FloatParam(float _val, float _min, float _max)
		: val(_val), min(_min), max(_max)
	{

	}

	FloatParam() 
	{

	}

};


struct Vec3Param
{
	glm::vec3 val;
	glm::vec3 min;
	glm::vec3 max;

	Vec3Param(glm::vec3 _val, glm::vec3 _min, glm::vec3 _max)
		: val(_val), min(_min), max(_max)
	{

	}

	Vec3Param()
	{

	}
};

struct Vec2Param
{
	glm::vec2 val;
	glm::vec2 min;
	glm::vec2 max;

	Vec2Param(glm::vec2 _val, glm::vec2 _min, glm::vec2 _max)
		: val(_val), min(_min), max(_max)
	{

	}

	Vec2Param()
	{

	}
};


class Material
{
public:
	Material(std::string name, ShaderProgram* _shader);
	

	void setShader(ShaderProgram* _shader);

	ShaderProgram* getShader() { return shader; }

	void addReference(Entity* _entity);
	void removeReference(Entity* _entity);

	void registerUniforms();

	std::string getName() { return name; }


	std::unordered_map<std::string, Texture*> vTexture; // Texture slots
	std::unordered_map<std::string, FloatParam> vFloat; // Float parameters
	std::unordered_map<std::string, int> vInt; // Integer parameters
	std::unordered_map<std::string, Vec3Param> vVec3; // Vec3 parameters
	std::unordered_map<std::string, Vec2Param> vVec2; // Vec2 parameters
	std::unordered_map<std::string, Color3> vColor; // Color parameters

	std::vector<Entity*> entityRefs; // References to all entities using this material

	bool hasDiffuseTexture, hasNormalsTexture, hasSpecularTexture;
private:

	// Shader that the material uses
	ShaderProgram* shader;

	// Name of material
	std::string name;
};

#endif