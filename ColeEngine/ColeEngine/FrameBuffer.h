#pragma once

#ifndef _FRAMEBUFFER
#define _FRAMEBUFFER

#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "glew.h"
#include "Texture.h"
#include "Shapes.h"

struct FboTexture
{
	Texture* texture = nullptr;
	std::string name;
};

class FrameBuffer
{
public:
	FrameBuffer(unsigned int _width, unsigned int _height);
	~FrameBuffer();

	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }
	unsigned int get() { return ID; }

	FullScreenQuad* getQuad() { return quad; };

	std::vector<FboTexture>& getTextures() { return textures; }

	void addTexture(std::string name);

private:
	unsigned int width, height;

	std::vector<FboTexture> textures;

	unsigned int ID, nAttachments;

	FullScreenQuad* quad;
};

#endif