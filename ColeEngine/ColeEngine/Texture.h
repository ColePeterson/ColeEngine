#pragma once

#ifndef _TEXTURE
#define _TEXTURE


#include <vector>
#include <iostream>
#include <string>
#include "glew.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Texture
{
public:
	Texture();
	Texture(unsigned int _width, unsigned int _height, unsigned int _nChannels, GLint filtering);
	~Texture();

	int getWidth() { return width; }
	int getHeight() { return height; }
	int numChannels() { return channels; }

	unsigned int get() { return texture; }

	bool load(std::string path);
	bool loadEmbedded(aiTexel* textureData, unsigned int _width, unsigned int _height);
	void unload();

	void bind();
	void unbind();
private:

	unsigned char* data;
	int width;
	int height;
	int channels;

	unsigned int texture;
};

#endif