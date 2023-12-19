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

out vec3 normalVec, tanVec, lightVec, eyeVec;
out vec2 texCoord;
uniform vec3 lightPos;

uniform mat4 shadowMatrix;

out vec4 shadowCoord;

void main()
{
    vec3 eye = (WorldInverse*vec4(0,0,0,1)).xyz;

    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    
    vec3 worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(inverse(mat3(ModelTr))); 
    tanVec = cross(normalVec, vec3(0., 1., 0.));
    lightVec = lightPos - worldPos;
    eyeVec = eye - worldPos;

    shadowCoord = shadowMatrix * ModelTr * vertex;

    texCoord = vertexTexture; 
}
