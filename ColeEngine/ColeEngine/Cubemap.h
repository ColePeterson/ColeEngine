#pragma once

#ifndef _CUBEMAP
#define _CUBEMAP

#include<iostream>



class Cubemap
{
public:
	Cubemap(unsigned int _width, unsigned int _height);
	~Cubemap();

	unsigned int get() { return cubemapID; }
	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }

	void bind();
	void unbind();
private:
	unsigned int cubemapID;
	unsigned int width, height;
};

#endif