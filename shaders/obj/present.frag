#version 450


//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inViewPos;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inUV;
layout (location = 5) in vec3 inlightDir;

//output write
layout (location = 0) out vec4 outFragColor;




vec3 rgbConvert(vec3 rgb)
{
    return rgb/255.0;
}

void main()
{
    vec3 tipColor = rgbConvert(vec3(242.0,175.0,239.0));
    vec3 c1 = rgbConvert(vec3(196.0,153.0,243.0));
    vec3 c2 = rgbConvert(vec3(115.0,96.0,223.0));

    vec3 AOColor = rgbConvert(vec3(51.0,24.0,107.0));

    vec3 tip = mix(vec3(0.0), tipColor, inUV.y * inUV.y);   
    vec3 ao = mix(AOColor, vec3(1.0f), inUV.y);

    vec3 color = mix(c2,c1,inUV.y);

    vec3 grasscolor = (color + tip)*ao;
    //grasscolor = pow(grasscolor,vec3(0.454));

    /* Fog */
    float _FogOffset = 10.0;
    float _FogDensity = 0.05;
    vec3 _FogColor = vec3(1.);
    float viewDistance = length(inViewPos - inFragPos)/10;
    float fogFactor = (_FogDensity / sqrt(log(2))) * (max(0.0f, viewDistance - _FogOffset));
    fogFactor = exp2(-fogFactor);
    fogFactor = max(0.5,fogFactor); 

    vec3 finalColor = mix(_FogColor,grasscolor,fogFactor);
    outFragColor = vec4(finalColor,0.0);
}