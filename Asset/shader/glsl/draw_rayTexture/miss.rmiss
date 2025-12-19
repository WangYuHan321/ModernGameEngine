#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
	//背景色
	hitValue = vec3(0.02,0.3, 0.06);
}