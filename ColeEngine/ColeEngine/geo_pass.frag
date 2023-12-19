#version 330 core

precision highp float;

layout (location = 0) out vec4 gVertex;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec4 gSpecular;


in vec2 texCoord;
in vec3 worldPos;
in vec3 normalVec;
in vec3 tanVec;
in float depth;

struct Color3
{
    vec3 c;
};

uniform sampler2D textureDiffuse;
uniform sampler2D textureNormal;
uniform sampler2D textureSpecular;


uniform vec2 textureScale;

uniform Color3 diffuse;
uniform Color3 specular;

uniform float shininess;
uniform float normalStrength;



//WORLD
uniform float time;
//WORLD_END


vec3 applyNormalMap(vec3 N, vec2 uv)
{
    vec3 d = 2.0*texture(textureNormal, uv).xyz - vec3(1);
    vec3 T = normalize(cross(vec3(0., 1., 0.), N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vec3 worldSpaceNormal = normalize(TBN * d);

    return normalize(N + worldSpaceNormal * -normalStrength);
}

void main()
{    
    vec2 uv = texCoord * textureScale;

    gVertex = vec4(worldPos, depth);
    
    gNormal = applyNormalMap(normalVec, uv);

    gAlbedo = diffuse.c * texture(textureDiffuse, uv).rgb;
    
    gSpecular = vec4(specular.c, shininess);

}  