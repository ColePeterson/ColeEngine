#include "Texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Texture::Texture()
	:data(nullptr)
{
	
}


// Constructor for creating empty texture
Texture::Texture (unsigned int _width, unsigned int _height, unsigned int _nChannels, GLint filtering)
	: data(nullptr), channels(_nChannels), width(_width), height(_height)
{
	// Generate texture ID and bind
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Create texture
	if (channels == 4)
	{
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (channels == 3)
	{
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, 0x1907, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

	glBindTexture(GL_TEXTURE_2D, 0);

}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}


void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::loadEmbedded(aiTexel* textureData, unsigned int _width, unsigned int _height)
{

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int components_per_pixel = 0;
	unsigned char* image_data = nullptr;

	if (_height == 0)
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(textureData), _width, &width, &height, &components_per_pixel, 0);
	}
	else
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(textureData), _width * _height, &width, &height, &components_per_pixel, 0);
	}

	std::cout << "ERROR: " << stbi_failure_reason() << "\n\n";

	if (image_data)
	{

		if (components_per_pixel == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
		}
		else if (components_per_pixel == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		}

		glGenerateMipmap(GL_TEXTURE_2D);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x2901);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x2901);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 0x2703);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 0x2601);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);

		//glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data.data());

		return true;
	}
	else
	{
		std::cout << "Failed to load embeddded texture\n\n";
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
}


bool Texture::load(std::string path)
{
	//stbi_set_flip_vertically_on_load(true);
	data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	if (data != nullptr)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x2901);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x2901);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 0x2703);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 0x2601);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
		if (channels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (channels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 0x1907, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);


		stbi_image_free(data);

		

		return true;
	}
	else
	{
		std::cout << "Failed to load texture\n\n";
		return false;
	}
}

void Texture::unload()
{
	glDeleteTextures(1, &texture);
}
