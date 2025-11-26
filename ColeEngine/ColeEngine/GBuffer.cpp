#include "GBuffer.h"
#include "Logging.h"

GBuffer::GBuffer(unsigned int _width, unsigned int _height)
	: width(_width), height(_height), quad(nullptr), nBuffers(0)
{

	// Create full screen quad to render G-Buffer to
	quad = new FullScreenQuad();

	if (!quad)
		Log::error("Could not create full screen quad for G-Buffer!");
	

	// Generate G-Buffer framebuffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	
	
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::warning("G-Buffer frame buffer not complete!");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



Texture* GBuffer::getTexture(BufferType type)
{
	// Check if texture with this type is part of G-Buffer
	auto it = textures.find(type);
	if (it != textures.end())
	{
		Texture* temp = textures[type];
		// Found it. It's valid. Return it.
		if (temp)
		{
			return temp;
		}
		else
		{
			Log::error("Couldnt get texture from G-Buffer. Texture does not exist.");
		}
	}
	else // Texture is not in G-Buffer textures 
	{
		Log::error("Couldnt get texture from G-Buffer. Texture type not in list.");
	}
	return nullptr;
}

void GBuffer::addBuffer(BufferType type)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Create texture for positions
	Texture* tex = new Texture(width, height, 4, GL_NEAREST);

	if (!tex)
		Log::error("Failed to create new texture for G-Buffer!");
	else
	{
		// Attach texture to framebuffer color out 0
		glBindTexture(GL_TEXTURE_2D, tex->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + nBuffers, GL_TEXTURE_2D, tex->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		nBuffers++;

		// Add to list of textures
		textures.emplace(type, tex);

		// Bind FBO
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// Set which color attachments are used for rendering on this framebuffer
		std::vector<unsigned int> attachments;
		for (unsigned int i = 0; i < nBuffers; i++)
		{
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		glDrawBuffers(nBuffers, attachments.data());

		// Check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			Log::warning("G-Buffer frame buffer not complete!");

	}

}



GBuffer::~GBuffer()
{
	// Delete textures attached to FBO
	for (const auto& pair : textures) 
	{
		if (pair.second)
		{
			delete pair.second;
		}
	}
	/*
	if (texPosition)
	{
		delete texPosition;
		texPosition = nullptr;
	}
	if (texNormal)
	{
		delete texNormal;
		texNormal = nullptr;
	}
	if (texAlbedo)
	{
		delete texAlbedo;
		texAlbedo = nullptr;
	}
	if (texSpecular)
	{
		delete texSpecular;
		texSpecular = nullptr;
	}
	*/
	if (quad)
	{
		delete quad;
		quad = nullptr;
	}
}