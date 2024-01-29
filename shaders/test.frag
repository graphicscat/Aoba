#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

vec3 RGBConvert(vec3 c)
{

  return vec3(c.x/255.0,c.y/255.0,c.z/255.0);

}

void main() 
{
  vec3 test = RGBConvert(vec3(255.0, 152.0, 67.0));
  outFragcolor = vec4(test, 1.0);	
}