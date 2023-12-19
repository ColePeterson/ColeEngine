#pragma once

#ifndef _TEXTURE
#define _TEXTURE



#include <iostream>
#include <string>
#include "glew.h"

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