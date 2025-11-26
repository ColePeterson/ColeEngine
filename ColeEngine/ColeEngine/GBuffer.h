#pragma once


#ifndef _GBUFFER
#define _GBUFFER

#include<unordered_map>
#include<string>

#include "Shapes.h"
#include "Texture.h"
#include "glew.h"

enum class BufferType
{
	POSITION,
	NORMALS,
	ALBEDO,
	SPECULAR,
	VIEW
};


class GBuffer
{
public:
	GBuffer(unsigned int _width, unsigned int _height);
	~GBuffer();

	// Add new atachment to FBO
	void addBuffer(BufferType type);

	// Get a texture attached to FBO
	Texture* getTexture(BufferType type);

	// Get FBO ID
	unsigned int getBuffer() { return gBuffer; }

	// Get screen quad
	FullScreenQuad* getQuad() { return quad; };

	// Get width and height
	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }

private:
	
	// Texture attackments for FBO
	std::unordered_map<BufferType, Texture*> textures;

	unsigned int nBuffers; // Number of textures on this FBO
	unsigned int gBuffer; // FBO ID
	unsigned int width, height; // Buffer width/height

	// Full screen quad mesh to render to
	FullScreenQuad* quad;
};


#endif