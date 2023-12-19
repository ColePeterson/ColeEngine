// Vertex shader for point light pass

#version 330 core

precision highp float;

//layout (location = 0) in vec4 vertex;
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;


uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

//out vec3 FragPos;

void main()
{
    vec4 v = vec4(vertex.x, -vertex.z, vertex.y, vertex.w);
    gl_Position = ModelTr * vertex;
    //FragPos = (ModelTr*vertex).xyz;
    //gl_Position = WorldProj*WorldView*ModelTr*vertex;

}  