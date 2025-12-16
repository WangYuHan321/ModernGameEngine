#version 460

//最终的击中着色器
#extension GL_EXT_ray_tracing : enable     //Vulkan GLSL 着色器扩展声明，用于启用光线追踪和图像加载功能。
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
	hitValue = vec3(0.02, 0.3, 0.02);
}