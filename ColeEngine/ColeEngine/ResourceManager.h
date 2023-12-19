#pragma once

#ifndef _RESOURCEMANAGER
#define _RESOURCEMANAGER


#include<string>
#include<vector>
#include<unordered_map>

#include "Texture.h"
#include "Shader.h"
#include "Material.h"

class ResourceManager
{
public:
	ResourceManager();

	ShaderProgram* shader(std::string name);

	Texture* getTexture(std::string src);

	Material* getMaterial(std::string name);

	// Load shader program (fragment, vertex, geometry)
	void loadShader(std::string name, std::string fragSrc, std::string vertSrc, std::string geomSrc);

	// Load shader program (fragment, vertex)
	void loadShader(std::string name, std::string fragSrc, std::string vertSrc);


	void createNewMaterial(std::string name, ShaderProgram* shader);

	void freeShader(std::string name);
	void freeTexture(std::string name);

	std::unordered_map<std::string, ShaderProgram*> shaders;
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, Material*> materials;

	Texture* getSkyTexture() { return skyTexture; }
	void setSkyTexture(Texture* _texture) { skyTexture = _texture; }

private:
	
	Texture* loadTexture(std::string src);

	Texture* skyTexture;
};

#endif