#version 330 core

precision highp float;

layout (location = 0) out vec4 gVertex;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gView;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normalVec;
in vec3 eyeVec;
in vec3 tanVec;
in float depth;

struct Color4
{
    vec4 c;
};

uniform sampler2D textureDiffuse;
uniform sampler2D textureNormal;
uniform sampler2D textureSpecular;


uniform vec2 textureScale;

uniform Color4 diffuse;
uniform Color4 specular;

uniform float shininess;
uniform float reflectivity;
uniform float normalStrength;



//WORLD
uniform float time;

uniform bool hasDiffuseTexture;
uniform bool hasNormalsTexture;
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
    
    if(hasNormalsTexture)
    {
        gNormal = applyNormalMap(normalVec, uv);
    }
    else
    {
        gNormal = normalVec;
    }


    if(hasDiffuseTexture)
    {
        gAlbedo.xyz = diffuse.c.xyz * texture(textureDiffuse, uv).xyz;
        gAlbedo.w = diffuse.c.w;
    }
    else
    {
        gAlbedo.xyz = diffuse.c.xyz;
        gAlbedo.w = diffuse.c.w;
    }

    gView = vec4(eyeVec, 1.0);
    
    gSpecular = vec4(specular.c.xyz, shininess);

}  