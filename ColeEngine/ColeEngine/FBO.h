#pragma once

#ifndef _FBO
#define _FBO

#include<string>

#include"Texture.h"
#include"Cubemap.h"

class FBO 
{
public:
    FBO();
    ~FBO();
    unsigned int fboID;

    Texture* texture2D;
    Cubemap* textureCube;

    int width, height;  // Size of the texture.

    void CreateFBO_2D(const int w, const int h, bool isShadowMap);
    void CreateFBO_3D(const int w, const int h, bool isShadowMap);


    // Bind this FBO to receive the output of the graphics pipeline.
    void BindFBO();

    // Unbind this FBO from the graphics pipeline;  graphics goes to screen by default.
    void UnbindFBO();

    // Bind this FBO's texture to a texture unit.
    void BindTexture(const int unit, const int programId, const std::string& name);

    // Unbind this FBO's texture from a texture unit.
    void UnbindTexture(const int unit);
};

#endif