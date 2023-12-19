#pragma once


#ifndef _GBUFFER
#define _GBUFFER

#include "Shapes.h"
#include "Texture.h"
#include "glew.h"

class GBuffer
{
public:
	GBuffer(unsigned int _width, unsigned int _height);
	~GBuffer();

	Texture* getPosition() { return texPosition;}
	Texture* getNormal() { return texNormal;}
	Texture* getAlbedo() { return texAlbedo;}
	Texture* getSpecular() { return texSpecular; }
	//Texture* getDepth() { return texDepth; }

	unsigned int getBuffer() { return gBuffer; }

	FullScreenQuad* getQuad() { return quad; };

	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }

private:
	Texture* texPosition;
	Texture* texNormal;
	Texture* texAlbedo;
	Texture* texSpecular;
	//Texture* texDepth;

	FullScreenQuad* quad;

	unsigned int gBuffer;

	unsigned int width, height;
};


#endif