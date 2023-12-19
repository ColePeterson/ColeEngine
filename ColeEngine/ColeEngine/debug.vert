/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

//out float d1;
//out float d2;

void main()
{
    vec4 pos = WorldProj*WorldView*ModelTr*vertex;

    //d1 = pos.w;
    //d2 = vertex.w;

    gl_Position = pos;
    
}
