#version 330 core

precision highp float;


out vec4 FragColor;

in vec2 texCoord;

uniform float time;

uniform sampler2D sceneColor;



void main()
{ 
    vec2 uv = texCoord;

   
    vec3 col = texture(sceneColor, uv).rgb;

    //col = pow(col*1.5, vec3(2.5));
    
    col = 1.-exp(-col);
    FragColor = vec4(col, 1.0);

}