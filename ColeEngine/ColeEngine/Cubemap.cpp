#include "Cubemap.h"

#include "glew.h"


Cubemap::Cubemap(unsigned int _width, unsigned int _height)
	: width(_width), height(_height)
{
	// Generate texture ID
	glGenTextures(1, &cubemapID);

	// Bind texture as cubemap
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

	// Generate cubemap
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	


}

void Cubemap::bind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
}
void Cubemap::unbind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &cubemapID);
}