#version 330 core

precision highp float;

layout(location = 0) in vec4 vertex; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexTexture; 
layout(location = 3) in vec3 vertexTangent; 

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

out vec3 normalVec, tanVec, lightVec, eyeVec, eyePos, worldPos;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform vec2 offset;
uniform vec4 color;

void main()
{
    //float scale = 10.0f;
    TexCoords = vertexTexture;
    ParticleColor = color;

    vec4 p = WorldProj*WorldView*ModelTr*vertex;

    gl_Position = p;

}