#version 330 core

precision highp float;

out vec4 FragColor;
  
in vec2 texCoord;

const float pi = 3.14159;

struct Light 
{
    vec3 pos;
    vec3 color;
    float strength;
    float falloff;
    float farPlane;
};

// Number of point lights
const int NR_LIGHTS = 2;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform sampler2D skyTexture; // 4

uniform samplerCube shadowMapPoint[NR_LIGHTS]; // 4, 5

uniform Light lights[NR_LIGHTS];

uniform bool basicShading;

uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 sunPos;


//uniform float metallic;
//uniform float roughness;
//uniform float ao;

const float metallic = 0.5;
const float roughness = 0.5;
const float ao = 0.5;

float pointShadowCalculation(vec3 worldPos, vec3 pos, float far, int i)
{
    vec3 fragToLight = worldPos - pos;

    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(shadowMapPoint[i], fragToLight).r;

    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}


vec3 lighting_pointLights(vec3 worldPos, vec3 normal, vec3 albedo, vec4 specular)
{
    vec3 n = normalize(normal);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 lighting = ambientColor;

    for(int i = 0; i < NR_LIGHTS; i++)
    {
        float pointShadow = pointShadowCalculation(worldPos, lights[i].pos,  lights[i].farPlane, i);  

        // diffuse
        vec3 lightDir = normalize(lights[i].pos - worldPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float lightDist = length(lights[i].pos - worldPos);
        float falloff = lights[i].strength / pow(lightDist, lights[i].falloff);

        float dif = max(dot(n, lightDir), 0.0);
        float spec = pow(max(dot(n, halfwayDir), 0.0), specular.w);

        vec3 objCol = (1.0 - pointShadow) * (dif + spec*specular.rgb) * albedo * falloff * lights[i].color; 
        //vec3 objCol = (ambientColor + (1.0 - pointShadow) * (dif + spec*Specular.rgb)) * Albedo * falloff * lights[i].color; 
        lighting += objCol;
    }
    
    lighting /= float(NR_LIGHTS);

    return 1.0 - exp(-lighting);
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 lighting_pointLights_BRDF(vec3 worldPos, vec3 normal, vec3 albedo, vec4 specular)
{
    vec3 N = normalize(normal);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 lighting = ambientColor;

    vec3 Lo = vec3(0.0);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    for(int i = 0; i < NR_LIGHTS; i++)
    {
        float pointShadow = pointShadowCalculation(worldPos, lights[i].pos,  lights[i].farPlane, i);  

        vec3 lightDir = normalize(lights[i].pos - worldPos);
        float lightDist = length(lights[i].pos - worldPos);
        vec3 H = normalize(viewDir + lightDir);
        float falloff = lights[i].strength / pow(lightDist, lights[i].falloff);
        vec3 radiance = lights[i].color * falloff;

        vec3 F  = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

        float NDF = DistributionGGX(N, H, roughness);       
        float G   = GeometrySmith(N, viewDir, lightDir, roughness);   

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, viewDir), 0.0) * max(dot(N, lightDir), 0.0)  + 0.0001;
        vec3 specular     = numerator / denominator;  

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
  
        kD *= 1.0 - metallic;	

        float NdotL = max(dot(N, lightDir), 0.0);        
        Lo += (kD * albedo / pi + specular) * radiance * NdotL;

    }
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;  

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    
    return 1.0 - exp(-lighting);
}


vec3 lighting_phong(vec3 worldPos, vec3 normal, vec3 albedo, vec4 specular)
{
    vec3 n = normalize(normal);
    vec3 viewDir = normalize(viewPos - worldPos);
    
    vec3 lightCol = vec3(1., 1., 1.);
    vec3 ld = normalize(sunPos);
    vec3 halfwayDir = normalize(ld + viewDir);

    float dif = max(dot(n, ld), 0.1);
    float spec = pow(max(dot(n, halfwayDir), 0.0), specular.w);

    return (ambientColor + (dif + spec*specular.rgb)) * albedo; 

}
void main()
{   
    // Get data from G-buffer
    vec4 worldPos = texture(gPosition, texCoord);
    vec3 Normal = texture(gNormal, texCoord).rgb;
    vec3 Albedo = texture(gAlbedo, texCoord).rgb;
    vec4 Specular = texture(gSpecular, texCoord);
    float depth = worldPos.w;

    vec3 col = vec3(0);

    if(basicShading)
    {
        col = lighting_phong(worldPos.xyz, Normal, Albedo, Specular);
       // col = Albedo;
    }
    else
    {
        col = lighting_pointLights(worldPos.xyz, Normal, Albedo, Specular);
    }
    
    

    vec3 rd = normalize(viewPos - worldPos.xyz); // Ray direction
    vec2 tc = vec2(-atan(rd.y, rd.x)/(2.*pi), acos(-rd.z) / pi);

    if(depth > 500.0)
        col = texture(skyTexture, tc).xyz;

    FragColor = vec4(col, 1.0);
}  