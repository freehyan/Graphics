#version 450 core

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

uniform float uShininess = 128;
uniform float uSpecularIntensity = 1.0;

uniform sampler2D uWorldPosTex;
uniform sampler2D uColorTex;
uniform sampler2D uNormalTex;
uniform vec3 uEyeWorldPos;
uniform vec2 uScreenSize;

out vec4 FragColor;

//******************************************************************
//FUNCTION:
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

//******************************************************************
//FUNCTION:
vec2 calcTexCoords()
{
	return gl_FragCoord.xy / uScreenSize;
}

void main()
{
	vec2 TexCoords = calcTexCoords();
	vec3 WorldPos = texture2D(uWorldPosTex, TexCoords).xyz;
	vec3 Color    = texture2D(uColorTex, TexCoords).xyz;
	vec3 Normal   = texture2D(uNormalTex, TexCoords).xyz;
	Normal = normalize(Normal);

//	FragColor = 0.01 * vec4(Color, 1.0) + calcDirectionalLight(WorldPos, Normal); 
	FragColor = vec4(Color, 1.0) * calcDirectionalLight(WorldPos, Normal);
}