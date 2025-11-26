#version 330 core

precision highp float;

in vec2 TexCoords;
in vec4 ParticleColor;

layout (location = 0) out vec3 gVertex;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gView;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normalVec;
in vec3 eyeVec;
in vec3 tanVec;

uniform vec4 diffuse;

uniform sampler2D sprite;

void main()
{
   
    vec4 tex = texture(sprite, TexCoords);
    float val = smoothstep(0.4, 0.0, length(TexCoords - 0.5));
    if(val < 0.001)
        val = 0.0;
        
    gAlbedo.xyz = diffuse.xyz;
    gAlbedo.w = val * diffuse.w;

    //gAlbedo = (texture(sprite, TexCoords) * ParticleColor);

    gNormal = normalVec;
    gVertex = worldPos;
    gSpecular = vec4(0.0, 0.0, 0.0, 1.0);
    
}  