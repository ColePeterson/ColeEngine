#version 330 core

precision highp float;

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

//out vec4 FragColor;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
    //gl_FragDepth = FragPos.x;
    //gl_FragDepth = 10.0;
    //FragColor = vec4(0., 1., 1., 1.);
    //gl_FragData[0] = vec4(1);
}  