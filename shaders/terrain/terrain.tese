#version 450

layout(set = 0, binding = 0) uniform  CameraBuffer{   
	mat4 proj;    
	mat4 view;
	vec4 viewPos; 
	//vec4 lightPos;
} cameraData;

layout(binding = 1) uniform sampler2D heightMap;

layout(binding = 3) uniform sampler2D normalMap;

layout(location = 0) in vec2 inUV[];

layout(quads, equal_spacing, cw) in;

layout(location = 0) out vec2 outUV;

layout(location = 1)out float outHeight;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 c1 = mix(inUV[0],inUV[1],u);
    vec2 c2 = mix(inUV[3],inUV[2],u);

    outUV = mix(c1,c2,v);

    vec3 normalS = texture(normalMap,outUV).rgb;
    normalS = normalS*2.0 - 1.0;

    normalS = normalize(normalS);

    vec3 T = vec3(1.0,0.0,0.0);
    vec3 B = vec3(0.0,0.0,-1.0);
    vec3 N = vec3(0.0,1.0,0.0);

    mat3 TBN = mat3(T,B,N);

    normalS = normalize(TBN * normalS);

    // Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

    vec4 uVec = gl_in[1].gl_Position - gl_in[0].gl_Position;
    vec4 vVec = gl_in[3].gl_Position - gl_in[0].gl_Position;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    

    outHeight = texture(heightMap,outUV).r*30;

    //pos += vec4(normal)*outHeight;
    pos.y = outHeight;

    gl_Position = cameraData.proj * cameraData.view  * pos;

}