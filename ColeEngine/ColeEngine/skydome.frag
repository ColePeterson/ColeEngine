
#version 330



out vec4 FragColor;

float pi = 3.14159;

in vec3 eyeVec;



//uniform sampler2D textureSky; // 0
uniform sampler2D textureDiffuse;

void main()
{
    vec3 rd = normalize(eyeVec); // Ray direction

    vec2 tc = vec2(-atan(rd.y, rd.x)/(2.*pi), acos(-rd.z) / pi);

    FragColor.xyz = texture(textureDiffuse, tc).xyz;

    return;
}
