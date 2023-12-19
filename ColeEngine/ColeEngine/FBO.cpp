
#include "glew.h"

//#include "Logging.h"
#include "fbo.h"




FBO::FBO() 
    : texture2D(nullptr), textureCube(nullptr)
{

}

FBO::~FBO()
{
    if (texture2D)
    {
        delete texture2D;
    }

    if (textureCube)
    {
        delete textureCube;
    }
}

void FBO::CreateFBO_2D(const int w, const int h, bool isShadowMap)
{
    width = w;
    height = h;

    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    // Create a render buffer, and attach it to FBO's depth attachment
   // unsigned int depthBuffer;
   // glGenRenderbuffers(1, &depthBuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
   // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    
    texture2D = new Texture(width, height, 4, GL_LINEAR);

    if (texture2D)
    {
        texture2D->bind();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        if (isShadowMap)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
        }
       

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2D->get(), 0);

        // Check for completeness/correctness
       // int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
        //if (status != int(GL_FRAMEBUFFER_COMPLETE))
       // {
           // printf("FBO Error: %d\n", status);
       // }
           

        // Unbind the fbo until it's ready to be used
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        texture2D->unbind();
    }
    else
    {
        //Log::error("Failed to create FBO 2D texture!");
    }
}


void FBO::CreateFBO_3D(const int w, const int h, bool isShadowMap)
{

    width = w;
    height = h;

    glGenFramebuffers(1, &fboID);

    glBindFramebuffer(GL_FRAMEBUFFER, fboID);


    // Create new cubemap texture
    textureCube = new Cubemap(w, h);

    if (textureCube)
    {
        textureCube->bind();

        // Attach cubemap to FBO depth attachment
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureCube->get(), 0);
       // checkGLError();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        //checkGLError();

        int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != int(GL_FRAMEBUFFER_COMPLETE))
        {
            printf("FBO Error: %d\n", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        textureCube->unbind();
    }
    else
    {
        //Log::error("Failed to create FBO cubemap texture!");
    }
}




void FBO::BindFBO() { glBindFramebuffer(GL_FRAMEBUFFER, fboID); }
void FBO::UnbindFBO() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FBO::BindTexture(const int unit, const int programId, const std::string& name)
{
    if (texture2D)
    {
        glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
        glBindTexture(GL_TEXTURE_2D, texture2D->get());
        int loc = glGetUniformLocation(programId, name.c_str());
        glUniform1i(loc, unit);
    }
    else
    {
       // Log::warning("Can't bind FBO texture because it doesn't exist");
    }
}

void FBO::UnbindTexture(const int unit)
{
    glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
    glBindTexture(GL_TEXTURE_2D, 0);
}
