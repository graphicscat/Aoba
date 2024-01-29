#version 450

#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform Input {
	vec3 iResolution;
	float iTime;
	float iTimeDelta;
	float iFrameRate;
	float iFrame;
	vec4 iMouse;
	vec4 iDate;
} ipt;

vec3 iResolution = ipt.iResolution;
float iTime = ipt.iTime;
float iTimeDelta = ipt.iTimeDelta;
float iFrameRate = ipt.iFrameRate;
float iFrame = ipt.iFrame;
vec4 iMouse = ipt.iMouse;
vec4 iDate = ipt.iDate;

layout(location = 0) out vec4 out_color;

#include "editor.glsl"

void mainImage( out vec4 fragColor, in vec2 fragCoord );

void main() {
	vec2  fragCoord= inUV*iResolution.xy;
	mainImage(out_color, fragCoord);
}