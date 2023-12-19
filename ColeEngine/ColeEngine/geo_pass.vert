#version 330

precision highp float;

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

layout(location = 0) in vec4 vertex; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexTexture; 
layout(location = 3) in vec3 vertexTangent; 

out vec3 normalVec, tanVec, lightVec, eyeVec, eyePos, worldPos;
out vec2 texCoord;

uniform vec3 lightPos;
uniform mat4 shadowMatrix;

out vec4 shadowCoord;

out float depth;

void main()
{
    vec3 eye = (WorldInverse*vec4(0,0,0,1)).xyz;

    //vec4 v = vec4(vertex.x, -vertex.z, vertex.y, vertex.w);
    vec4 p = WorldProj*WorldView*ModelTr*vertex;

    gl_Position = p;
    
    depth = p.w;

    eyePos = eye;

    worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(inverse(mat3(ModelTr))); 
    tanVec = cross(normalVec, vec3(0., 1., 0.));
    lightVec = lightPos - worldPos;
    eyeVec = eye - worldPos;

    
    texCoord = vertexTexture; 
}