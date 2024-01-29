#version 450

//layout (location = 0) in vec2 pos;

layout (push_constant) uniform Size {
    float width;
    float height;
} window_size;

layout (location = 0) out vec2 fragCoord;
//layout (location = 0) out vec2 outUV;

void main() {
    // gl_Position = vec4(pos, 0, 1);
    // fragCoord = ((pos + 1) / 2) * vec2(window_size.width, window_size.height);

    fragCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(fragCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    fragCoord.y = 1-fragCoord.y;
    
}
