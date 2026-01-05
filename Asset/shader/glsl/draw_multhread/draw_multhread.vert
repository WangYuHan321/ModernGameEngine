#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(std140, push_constant) uniform PushConsts
{
	mat4 mvp;
	vec3 color;
}pushConsts;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 3) out vec3 outViewVec;
layout(location = 4) out vec3 outLightVec;

void main()
{
	outColor = inColor;
	
	gl_Position = pushConsts.mvp * vec4(inPos, 1.0);
	
	vec4 pos = pushConsts.mvp * vec4(inPos, 1.0);
	outNormal = mat3(pushConsts.mvp) * inNormal;
	vec3 lPos = vec3(4.0, 8.0, 1.0);
	outLightVec = lPos - pos.xyz;
    outViewVec = -pos.xyz;
}
