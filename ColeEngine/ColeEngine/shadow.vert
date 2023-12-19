
#version 330

uniform mat4 WorldView, WorldProj, ModelTr;

uniform vec3 lightPos;

in vec4 vertex;

out vec4 position;

void main()
{      
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    position = gl_Position;
}
