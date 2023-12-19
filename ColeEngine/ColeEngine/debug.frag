
#version 330

out vec4 FragColor;

in float d1;
in float d2;

uniform vec3 diffuse;



void main()
{
    vec3 col = diffuse;

    //if(d1 < d2)
        //col = vec3(1., 0., 0.);

    FragColor.xyz = col;
    return;
}
