
#version 330

out vec4 FragColor;

const int     nullId	= 0;
const int     skyId	= 1;
const int     testId	= 2;
const int     groundId	= 3;
const int     treeId	= 4;
const int     debugId	= 5;

float pi = 3.14159;
float pi2 = 2*pi;

in vec3 normalVec, tanVec, lightVec, eyeVec;
in vec2 texCoord;
in vec4 shadowCoord;

uniform int objectId;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

uniform float time;

uniform sampler2D textureDiffuse; // 0
uniform sampler2D textureNormal; // 1
uniform sampler2D textureSky; // 2
uniform sampler2D shadowMap; // 3

vec3 incomingHdrLight(vec3 n)
{
     vec2 tc = vec2(-atan(n.y, n.x)/(2.*pi), acos(n.z) / pi);
     return texture(textureSky, tc).xyz;
}

float checkers(vec2 p, float k)
{
    vec2 id = floor(k*texCoord.xy);
    return mod(id.x + id.y, 2.0);
}


vec3 applyNormalMap(vec3 N, vec2 uv)
{
    vec3 d = (texture(textureNormal, uv).xyz*2.) - vec3(1);
    vec3 T = normalize(tanVec);
    vec3 B = normalize(cross(T, N));

    N = normalize(d.x*T + d.y*B + d.z*N);
    return N;
}

// Is vec2 between 0 - 1
bool inRange01(vec2 p)
{
    if((p.x >= 0. && p.x <= 1.) && (p.y >= 0. && p.y <= 1.))
        return true;
    else
        return false;
}

uniform float debugFloat;

void main()
{
    if (objectId == debugId) {
        FragColor.xyz = diffuse;
        return; 
    }
    vec3 col = vec3(0);

    vec2 uv = vec2(-texCoord.x, texCoord.y);

    vec3 n = normalize(normalVec); // Normal vector

    n = applyNormalMap(n, uv*5.0);

    vec3 ld = normalize(lightVec); // Light direction
    vec3 rd = normalize(eyeVec); // Ray direction

    vec3 H = normalize(ld+rd);
    float NV = max(dot(n,rd),0.0);
    float HN = max(dot(H,n),0.0);

    float dif = max(dot(n,ld),0.0); // Diffuse value
    float spec = pow(HN,shininess); // Specular value

    //vec2 tuv = gl_FragCoord.xy / vec2(1920., 1080.);
    //FragColor.xyz = .03*texture(shadowMap, tuv).xyz;
    //return;

    vec3 objCol = diffuse;

    if(objectId == debugId)
    {
        FragColor.xyz = diffuse;
        return;
    }
    else if(objectId == groundId)
    {
        objCol = texture(textureDiffuse, uv*5.0).xyz;
    }
    else if (objectId==treeId) 
    {
        objCol *= max(.6, checkers(uv, 70.0));
    }
    else if (objectId==testId) 
    {
        objCol = 0.5 + 0.5*cos(4.*time+ 10.*uv.xyx+vec3(0.,2.,4.) + uv.x*4.) * cos(uv.y*10. + time);
    }

    vec3 irradianceColor = incomingHdrLight(n);
    objCol = (objCol / pi) * irradianceColor;

    col += 2.0*objCol;
    col += .3 * irradianceColor;
    col += objCol * dif;
    col += 0.6*specular*spec;


    vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;
    float lightDepth = texture(shadowMap, shadowIndex, 2.).w;
    float pixelDepth = shadowCoord.w;

    //float shdw = smoothstep(-debugFloat, debugFloat, abs(pixelDepth - lightDepth));
    //col.xyz *= max(shdw, .25);

    // Pixel lies in the the shadow map frustum
    if(shadowCoord.w > 0. && inRange01(shadowIndex) == true)
    {

        // Pixel is in shadow
        if(pixelDepth > lightDepth + .022)
        {
            col.xyz *= .5; // Darken image a bit instead of setting to ambient term. 
            //FragColor.xyz = ambient; // or do all of this at the beginning and set color to ambient term and return.
            //return;
        }
    }

   
    if(objectId == skyId)
    {
        vec2 tc = vec2(-atan(rd.y, rd.x)/(2.*pi), acos(-rd.z) / pi);
        col = texture(textureSky, tc).xyz;
    }

    

    

   
    FragColor.xyz = col;
}
