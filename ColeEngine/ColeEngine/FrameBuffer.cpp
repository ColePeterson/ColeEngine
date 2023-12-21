#include "FrameBuffer.h"
#include "Logging.h"
FrameBuffer::FrameBuffer(unsigned int _width, unsigned int _height)
	: width(_width), height(_height), ID(-1), nAttachments(0)
{
	// Generate G-Buffer framebuffer
	glGenFramebuffers(1, &ID);
	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	// Create texture for positions
	Texture* tex = new Texture(width, height, 4, GL_NEAREST);

	if (!tex)
		Log::error("Failed to create texture for framebuffer");
	else
	{
		// Attach texture to framebuffer color out 0
		glBindTexture(GL_TEXTURE_2D, tex->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + nAttachments, GL_TEXTURE_2D, tex->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		FboTexture fboTex = { tex, "texture name" };
		textures.push_back(fboTex);

		nAttachments++;
	}

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);


	// Create full screen quad to render framebuffer to
	quad = new FullScreenQuad();

	if (!quad)
		Log::error("Could not create full screen quad for framebuffer!");

}
FrameBuffer::~FrameBuffer()
{
	for (FboTexture tex : textures)
	{
		if (tex.texture)
		{
			delete tex.texture;
			tex.texture = nullptr;
		}
	}

	textures.clear();
}

void FrameBuffer::addTexture(std::string name)
{

	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	// Create texture for positions
	Texture* tex = new Texture(width, height, 4, GL_NEAREST);

	if (!tex)
		Log::error("Failed to create texture for framebuffer");
	else
	{
		// Attach texture to framebuffer color out 0
		glBindTexture(GL_TEXTURE_2D, tex->get());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + nAttachments, GL_TEXTURE_2D, tex->get(), 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		FboTexture fboTex = { tex, name };
		textures.push_back(fboTex);

		nAttachments++;

		// Set which color attachments are used for rendering on this framebuffer

		/*
		std::vector<unsigned int> att;
		for (int i = 0; i < nAttachments; i++)
		{
			att.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		//unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(nAttachments, att.data());
		*/
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
