#version 450

struct light
{
	vec4 position;  // position.w represents type of light
	vec4 color;     // color.w represents light intensity
	vec4 direction; // direction.w represents range
	vec4 info;      // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

layout(set = 0, binding = 1) uniform UniformBufferObject
{
	mat4 shadowmapSpace;
	mat4 localToWorld;
	vec4 cameraInfo;
	light directionalLights[16];
	light pointLights[512];
	light spotLights[16];
	ivec4 lightsCount; // [0] for directionalLights, [1] for pointLights, [2] for spotLights
	float zNear;
	float zFar;
};

layout(set = 0, binding = 2)  uniform sampler2D shadowmap;  // sky cubemap
layout(set = 0, binding = 3)  uniform sampler2D sampler1; // basecolor
layout(set = 0, binding = 4)  uniform sampler2D sampler2; // normal
layout(set = 0, binding = 5)  uniform sampler2D sampler3; // roughness

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragPositionWS;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
 vec3 VertexColor = fragColor;
 
 vec3 BaseColor = texture(sampler1, fragTexCoord).rgb;
 vec3 NormalColor = texture(sampler2, fragTexCoord).rgb;
 vec3 RoughnessColor = texture(sampler3, fragTexCoord).rgb;

 outColor = vec4(vec3(BaseColor), 1.0);
 
}
