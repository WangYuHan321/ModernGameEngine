#version 450 

layout (binding = 1) uniform sampler2DArray samplerArray;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout (location = 0) out vec4 outFragColor;

void main()
{
	vec4 color = texture(samplerArray, inUV);
	
	if(color.a < 0.5)
		discard;
		
	float specularStrength = 0.5;
	float shininess = 32.0;
	
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	
	// 计算反射向量
	vec3 R = reflect(-L, N);
	
	vec3 ambient = vec3(0.65);
	vec3 diffuse = max(dot(N, L), 0.0) * inColor;
	// 镜面反射（Blinn-Phong模型，效果更好）
	vec3 H = normalize(L + V); // 半角向量
	float spec = pow(max(dot(N, H), 0.0), shininess);
	vec3 specular = specularStrength * spec * vec3(1.0);// 光源 1.0
	
	outFragColor = vec4((ambient + diffuse + specular) * color.rgb, 1.0);
}
