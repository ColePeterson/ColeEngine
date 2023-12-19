
#version 330

precision highp float;

out vec4 FragColor;


struct Color3
{
    vec3 c;
};


float pi = 3.14159;



in vec3 normalVec, tanVec, lightVec, eyeVec, eyePos, fragPos;
in vec2 texCoord;
in vec4 shadowCoord;
in float dif;

// Object specific uniforms
uniform int objectId;

uniform float shininess;
uniform float normalStrength;
uniform vec2 textureScale;
uniform Color3 diffuse;
uniform Color3 specular;




// Debug values
uniform float debugFloat1;
uniform float debugFloat2;
uniform bool disableShadows;

//WORLD
uniform Color3 ambient;
uniform float time;
//WORLD_END

//NO_REGISTER
uniform float far_plane;
uniform vec3 lightPos;
//END_NO_REGISTER



uniform sampler2D textureDiffuse; // 0
uniform sampler2D textureNormal; // 1

uniform sampler2D textureSky; // 2
uniform sampler2D shadowMap; // 3
uniform samplerCube shadowMapPoint; // 4


vec3 incomingHdrLight(vec3 n){
     vec2 tc = vec2(-atan(n.y, n.x)/(2.*pi), acos(n.z) / pi);
     return texture(textureSky, tc).xyz;
}

float checkers(vec2 p, float k){
    vec2 id = floor(k*texCoord.xy);
    return mod(id.x + id.y, 2.0);
}


vec3 applyNormalMap_old(vec3 N, vec2 uv){
    vec3 d = 2.0*texture(textureNormal, uv).xyz - vec3(1);
    vec3 T = normalize(tanVec);
    vec3 B = normalize(cross(T, N));

    return mix(N, normalize(d.x*T + d.y*B + d.z*N), -normalStrength);
}

vec3 applyNormalMap(vec3 N, vec2 uv){
    vec3 d = 2.0*texture(textureNormal, uv).xyz - vec3(1);
    vec3 T = normalize(cross(vec3(0., 1., 0.), N));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vec3 worldSpaceNormal = normalize(TBN * d);

    return normalize(N + worldSpaceNormal * -normalStrength);
}

// Is vec2 between 0 - 1
bool inRange01(vec2 p){
    if((p.x >= 0. && p.x <= 1.) && (p.y >= 0. && p.y <= 1.))
        return true;
    else
        return false;
}


float pointShadowCalculation(vec3 worldPos)
{
    vec3 fragToLight = worldPos - lightPos;

    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(shadowMapPoint, fragToLight).r;
    //float closestDepth = 1.;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}



void main(){
    vec3 rd = normalize(eyeVec); // Ray direction
    vec3 ld = normalize(lightVec); // Light direction
    vec3 n = normalize(normalVec); // Normal vector
    vec3 H = normalize(ld+rd); // Half vector
    vec2 uv = textureScale * vec2(-texCoord.x, texCoord.y);
    
    n = applyNormalMap(normalVec, uv);

    
    float spec = pow(max(dot(n, H), 0.0), shininess); // Specular value

    vec3 objCol = diffuse.c * texture(textureDiffuse, uv).xyz; // Object base color
    
    vec3 col = vec3(0);
    
    /*
    // SHADOW
    float shdw = 0.0;
    // Directional shadows
    if(!disableShadows)
    {
        vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

        float pixelDepth = shadowCoord.w;
        float bias = max(0.05 * (1.0 - dot(n, ld)), 0.005);  

    
        int halfkernelWidth = 3;
        for(int x = -halfkernelWidth; x <= halfkernelWidth; ++x)
        {
            for(int y = -halfkernelWidth; y <= halfkernelWidth; ++y)
            {
                float lightDepth = texture(shadowMap, shadowIndex.xy + vec2(x, y) * texelSize).w; 
                shdw += pixelDepth - bias > lightDepth ? 1.0 : 0.0;
            }
        }

        shdw /= ((halfkernelWidth*2+1)*(halfkernelWidth*2+1));

        col = (ambient.c + (1.0 - shdw) * (dif + spec*specular.c)) * objCol; 
    }
    else
    {
        col = (ambient.c + (dif + spec*specular.c)) * objCol; 
    }
    */

    
    vec3 lightDir = normalize(lightPos - fragPos);
    float dif2 = max(dot(lightDir, n), 0.0);

    vec3 viewDir = normalize(eyeVec);
    vec3 reflectDir = reflect(-lightDir, n);
    float spec2 = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec2 = pow(max(dot(n, halfwayDir), 0.0), shininess);

    float dist = length(fragPos - lightPos);
    float falloff = 3.8 / (dist);

    float pointShadow = pointShadowCalculation(fragPos);  
    
    col = (ambient.c + (1.0 - pointShadow) * (dif2 + spec2*specular.c)) * objCol * falloff; 
   // col = (ambient.c + (dif2 + spec2*specular.c)) * objCol * falloff; 

    
   
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);  
    FragColor = vec4(col, 1.0);
}
