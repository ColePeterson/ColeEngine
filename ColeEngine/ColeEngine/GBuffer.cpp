#include "GBuffer.h"
#include "Logging.h"

GBuffer::GBuffer(unsigned int _width, unsigned int _height)
	: texPosition(nullptr), texNormal(nullptr), texAlbedo(nullptr),texSpecular(nullptr), 
	width(_width), height(_height), quad(nullptr)
{

	// Create full screen quad to render G-Buffer to
	quad = new FullScreenQuad();

	if (!quad)
		Log::error("Could not create full screen quad for G-Buffer!");
	


	// Generate G-Buffer framebuffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	



	// Create texture for positions
	texPosition = new Texture(width, height, 4, GL_NEAREST);

	if (!texPosition)
		Log::error("texPosition G-Buffer texture failed to create!");
	else
	{
		// Attach texture to framebuffer color out 0
		glBindTexture(GL_TEXTURE_2D, texPosition->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texPosition->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}



	// Create texture for normals
	texNormal = new Texture(width, height, 4, GL_NEAREST);

	if (!texNormal)
		Log::error("texNormal G-Buffer texture failed to create!");
	else
	{
		// Attach texture to framebuffer color out 1
		glBindTexture(GL_TEXTURE_2D, texNormal->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texNormal->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}



	// Create texture for Albedo
	texAlbedo = new Texture(width, height, 4, GL_NEAREST);

	if (!texAlbedo)
		Log::error("texAlbedo G-Buffer texture failed to create!");
	else
	{
		// Attach texture to framebuffer color out 2
		glBindTexture(GL_TEXTURE_2D, texAlbedo->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texAlbedo->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}



	// Create texture for specular color and power (r, g, b, power)
	texSpecular = new Texture(width, height, 4, GL_NEAREST);

	if (!texSpecular)
		Log::error("texSpecular G-Buffer texture failed to create!");
	else
	{
		// Attach texture to framebuffer color out 2
		glBindTexture(GL_TEXTURE_2D, texSpecular->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texSpecular->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	


	// Set which color attachments are used for rendering on this framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);




	


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



GBuffer::~GBuffer()
{
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
	if (quad)
	{
		delete quad;
		quad = nullptr;
	}
}