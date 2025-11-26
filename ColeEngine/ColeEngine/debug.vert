
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;



out VS_OUT 
{
    vec3 normal;
} vs_out;


void main()
{
    vec4 pos = WorldProj*WorldView*ModelTr*vertex;
    //vec4 pos = WorldView*ModelTr*vertex;

    vec3 norm = vec3(1.0, 0.0, 0.0);

    mat3 normalMatrix = mat3(transpose(inverse(WorldView * ModelTr)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * norm, 0.0)));

    gl_Position = pos;
    
}
