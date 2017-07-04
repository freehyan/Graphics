#version 450 core

uniform vec3 uLightSourceColor;
struct SBaseLight 
{
    vec4 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct SDirectionalLight
{
    SBaseLight BaseLight;
    vec4 Direction;
};
uniform SDirectionalLight uDirectionalLight;
uniform vec3 uEyeWorldPos;
uniform float uShininess = 128;
uniform float uSpecularIntensity = 1.0;

in vec3 g_WorldPos;
in vec3 g_Normal;

layout (location = 0) out vec4 FragColor_;

////******************************************************************
////FUNCTION:
vec4 calcLightInternal(in SBaseLight vBaseLight, in vec3 vLightDirection,  in vec3 vWorldPos, in vec3 vNormal)
{
	vec4 AmbientColor = vec4(vBaseLight.Color.xyz * vBaseLight.AmbientIntensity, 1.0);
	vec4 DiffuseColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 SpecularColor = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec3 LightDirection = normalize(-vLightDirection);
	float DiffuseFactor = dot(normalize(vNormal), LightDirection);
	if (DiffuseFactor > 0.0)
	{
		DiffuseColor = vec4(vBaseLight.Color.xyz * vBaseLight.DiffuseIntensity * DiffuseFactor, 1.0);
		
		vec3 Vertex2Eye = normalize(uEyeWorldPos - vWorldPos);
		vec3 ReflectLight = normalize(reflect(-LightDirection, normalize(vNormal)));	

		float Specularvactor = dot(Vertex2Eye, ReflectLight);
		if (Specularvactor > 0.0)
		{
			SpecularColor = vec4(pow(Specularvactor, uShininess) * vBaseLight.Color.xyz * uSpecularIntensity, 1.0);
		}
	}

	return AmbientColor + DiffuseColor + SpecularColor;
}

//******************************************************************
//FUNCTION:
vec4 calcDirectionalLight(in vec3 vWorldPos, in vec3 vNormal)
{
	return calcLightInternal(uDirectionalLight.BaseLight, uDirectionalLight.Direction.xyz, vWorldPos, vNormal);
}

void main()
{
	vec3 Normal = normalize(g_Normal);
	FragColor_ = vec4(uLightSourceColor, 1.0) * calcDirectionalLight(g_WorldPos, Normal);
	//FragColor_ = vec4(1.0, 0.0, 0.0, 1.0);
}