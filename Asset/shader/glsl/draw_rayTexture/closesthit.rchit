#version 460

#extension GL_EXT_ray_tracing : require //作用：启用光线追踪功能 用于光线追踪着色器（光线生成、相交、任何命中、最接近命中、未命中着色器）
#extension GL_GOOGLE_include_directive : require // 允许使用#include指令包含其他GLSL文件
#extension GL_EXT_nonuniform_qualifier : require // 允许使用nonuniformEXT修饰符 处理动态索引的纹理采样，避免编译器优化导致的错误
#extension GL_EXT_buffer_reference2 : require // 允许在着色器中定义缓冲区引用 直接访问GPU缓冲区，类似于指针
#extension GL_EXT_scalar_block_layout : require //启用标量块布局 允许更灵活的内存布局控制，减少填充 布局：layout(scalar)用于UBO/SSBO
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require //支持显式的64位整数类型

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(binding = 3, set = 0) uniform sampler2D image;

#include "bufferreferences.glsl"
#include "geometrytypes.glsl"

void main()
{
	Triangle tri = unpackTriangle(gl_PrimitiveID, 32);
	hitValue = vec3(tri.uv, 0.0f);
	// Fetch the color for this ray hit from the texture at the current uv coordinates
	vec4 color = texture(image, tri.uv);
	hitValue = color.rgb;
}