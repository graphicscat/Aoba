#version 450

layout( location = 0)in vec2 inUV;

layout( location = 1) in float height;

layout( location = 0) out vec4 color;

layout(binding = 2) uniform sampler2D terrainTex;

void main()
{
    vec3 terrainCol = texture(terrainTex,inUV).rgb;

    color = vec4(terrainCol,0.0);
}
