#version 450

#extension GL_GOOGLE_include_directive : enable

#define M_PI 3.141592654
#define SCALE 0.15

#include "simplex.glsl"

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 UV;

// Instanced attributes
layout (location = 4) in vec3 instancePos;
layout (location = 5) in vec2 instanceUV;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outViewPos;
layout (location = 2) out vec3 outFragPos;
layout (location = 3) out vec3 outNormal;
layout (location = 4) out vec2 outUV;
layout (location = 5) out vec3 outlightDir;

layout(set = 0, binding = 0) uniform  CameraBuffer{   
	mat4 proj;    
	mat4 view;
	vec4 viewPos; 
	//vec4 lightPos;
} cameraData;

layout(set = 0, binding = 1) uniform sampler2D windTex;

void sincos(float a, out float sino, out float coso)
{
    sino = sin(a);
    coso = cos(a);
}


vec4 RotateAroundYInDegrees (vec4 vertex, float degrees) {
    float alpha = degrees * M_PI / 180.0;
    float sina, cosa;
    sincos(alpha, sina, cosa);
    mat2 m = mat2(cosa, -sina, sina, cosa);
    return vec4(m*vertex.xz, vertex.yw).xzyw;
}

vec4 RotateAroundXInDegrees (vec4 vertex, float degrees) {
    float alpha = degrees * M_PI / 180.0;
    float sina, cosa;
    sincos(alpha, sina, cosa);
    mat2 m = mat2(cosa, -sina, sina, cosa);
    return vec4(m*vertex.yz, vertex.xw).zxyw;
}

void main() 
{	vec2 nUV = UV;
	nUV.y = 1-nUV.y;
	vec4 grassPosition = vec4(instancePos.xyz, 1.0);
	vec4 vertex4 = vec4(vPosition,1.0);
	float idHash = randValue(uint(abs(grassPosition.x * 10000 + grassPosition.y * 100 + grassPosition.z * 0.05 + 2)));
    idHash = randValue(uint(idHash * 100000));

	vec4 animationDirection = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    animationDirection = normalize(RotateAroundYInDegrees(animationDirection, idHash * 180.0f));

	vec4 localPosition = RotateAroundXInDegrees(vertex4, 90.0);
	//localPosition = RotateAroundYInDegrees(localPosition, idHash * 180.0);
	localPosition.y += SCALE * nUV.y * nUV.y * nUV.y;
	//localPosition.xz +=  mix(0.5, 1.0, idHash) * (nUV.y * nUV.y * SCALE) * animationDirection.xz;

	float swayVariance = mix(0.8, 1.0, idHash);
    float movement = nUV.y * nUV.y * (texture(windTex, instanceUV.xy).r);
    movement *= swayVariance;
    localPosition.xz += vec2(movement);

	//outlightDir = vec3(cameraData.lightPos);

	outFragPos = vec3(vec4(vPosition + instancePos.xyz, 1.0f));
	gl_Position =cameraData.proj*cameraData.view *(vec4(vPosition + instancePos.xyz, 1.0f)+localPosition);
	outColor = vec3(0.1,0.2,0.5);
	outViewPos = vec3(cameraData.viewPos);
	outNormal = normalize(vNormal);
	outUV = nUV;
}