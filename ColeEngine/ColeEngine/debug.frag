
#version 330

out vec4 FragColor;


uniform vec3 diffuse;



void main()
{
    vec3 col = diffuse;
   
    FragColor = vec4(col, 1.0);
    
    return;
}
