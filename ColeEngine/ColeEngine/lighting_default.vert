#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

layout(location = 0) in vec4 vertex; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexTexture; 
layout(location = 3) in vec3 vertexTangent; 

out vec3 normalVec, tanVec, lightVec, eyeVec, eyePos, fragPos;
out vec2 texCoord;
out float dif;

uniform vec3 lightPos;
uniform mat4 shadowMatrix;

out vec4 shadowCoord;

void main()
{
    vec3 eye = (WorldInverse*vec4(0,0,0,1)).xyz;

    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    
    eyePos = eye;

    vec3 worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(inverse(mat3(ModelTr))); 
    tanVec = cross(normalVec, vec3(0., 1., 0.));
    lightVec = lightPos - worldPos;
    eyeVec = eye - worldPos;
    fragPos = worldPos;

    shadowCoord = shadowMatrix * ModelTr * vertex;

    dif = max(dot(normalize(normalVec), normalize(lightVec)), 0.0);
    

    texCoord = vertexTexture; 
}
