#version 330 core

precision highp float;

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 vertexTexture;
out vec2 texCoord;



void main() 
{
    texCoord = vertexTexture;
    gl_Position = vec4(vertex.x, vertex.y, 0.0, 1.0);
}