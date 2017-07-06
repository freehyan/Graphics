#version 450

layout(location = 0) out vec4 _FluidColor;

struct SBaseLight
{
	vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	float SpecularIntensity;
	float SpecularPower;
};

struct SDirectionalLight
{
	SBaseLight BaseLight;
	vec3 Direction;
};

in vec2 g_TexCoord;

uniform sampler2D   uThicknessTex;
uniform sampler2D   uDepthTex;
uniform sampler2D   uNormalTex;
uniform sampler2D   uSceneTex;
uniform samplerCube uSkyBoxTex;
uniform vec2 uClipPosToEye;
uniform vec3 uEyeWorldPos;
uniform mat4 uWorldViewMatrixInverse;
uniform SDirectionalLight uDirectionalLight;

//--------------------------------------------------------------------------------------
// 转变法线
//--------------------------------------------------------------------------------------
vec3 transformNormalBack(in vec3 vNormal)
{
	return (vNormal-vec3(0.5)) * 2.0;
}

//--------------------------------------------------------------------------------------
// 屏幕空间转换到视图空间
//--------------------------------------------------------------------------------------
vec3 viewport2EyeSpace(in vec2 vCoord, in float vEyeZ)
{
	// find position at z=1 plane
	vec2 UV = (vCoord*2.0 - vec2(1.0)) * uClipPosToEye;

	return vec3(-UV * vEyeZ, vEyeZ);
}

//--------------------------------------------------------------------------------------
// 计算光照积分
//--------------------------------------------------------------------------------------
vec4 calcLightInternal(in SBaseLight vBaseLight, in vec3 vLightDirection, in vec3 vWorldPos, in vec3 vNormal)
{
	vec4 AmbientColor = vec4(vBaseLight.Color * vBaseLight.AmbientIntensity, 1.0);
	vec4 DiffuseColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 SpecularColor = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 LightDirection = normalize(-vLightDirection);
	float DiffuseFactor = dot(normalize(vNormal), LightDirection);
	if (DiffuseFactor > 0.0)
	{
		DiffuseColor = vec4(vBaseLight.Color * vBaseLight.DiffuseIntensity * DiffuseFactor, 1.0);

		vec3 Vertex2Eye = normalize(uEyeWorldPos - vWorldPos);
		vec3 ReflectLight = normalize(reflect(-LightDirection, normalize(vNormal)));

		float SpecularFactor = dot(Vertex2Eye, ReflectLight);
		if (SpecularFactor > 0.0)
		{
			SpecularColor = vec4(pow(SpecularFactor, vBaseLight.SpecularPower) * vBaseLight.Color * vBaseLight.SpecularIntensity, 1.0);
		}
	}

	return AmbientColor + DiffuseColor + SpecularColor;
}

//--------------------------------------------------------------------------------------
// 方向光
//--------------------------------------------------------------------------------------
vec4 calcDirectionalLight(in vec3 vWorldPos, in vec3 vNormal)
{
	return calcLightInternal(uDirectionalLight.BaseLight, uDirectionalLight.Direction, vWorldPos, vNormal);
}

void main(void)
{
//Get Texture Information about the Pixel
	vec3 f3Normal = texture2D(uNormalTex, g_TexCoord).xyz;
	f3Normal = normalize(transformNormalBack(f3Normal));
	float fThickness = texture2D(uThicknessTex, g_TexCoord).x;
	float fViewDepth =  texture2D(uDepthTex, g_TexCoord).x;

	vec3 f3ViewPos = viewport2EyeSpace(g_TexCoord, -fViewDepth);
	vec3 f3WorldPos = (uWorldViewMatrixInverse * vec4(f3ViewPos, 1.0f)).xyz;
	vec4 f4Color = texture2D(uSceneTex, g_TexCoord);

	if (fViewDepth > 0.0)
	{
		vec4 f4FluidColor = vec4(0.1f, 0.4f, 0.8f, 1.0f);
		fThickness = max(fThickness, 0.4);

//Fresnel Reflection -- Schlick's approximation(R(theta) = R0 + (1-R0) * (1-cos(theta))^5)
		float fFresnel = 0.1f + (1.0f - 0.1f) * pow(1.0f - max(dot(f3Normal, -normalize(f3ViewPos)), 0.0f), 5.0f);
		vec3 f3ViewDirection = normalize(f3WorldPos - uEyeWorldPos);
		vec3 f3Reflecttion = reflect(f3ViewDirection, f3Normal);
		vec3 f3SkyBoxColor = texture(uSkyBoxTex, f3Reflecttion).xyz;
		vec3 f3ReflectionColor = mix(vec3(0.0), f3SkyBoxColor, smoothstep(0.05f, 0.3f, f3WorldPos.y)); //????

//Refraction
		float fRadio = 1.0f / 1.33f; //空气和水的折射率
		vec3 f3Transmission = (1.0f - (1.0-f4FluidColor.xyz) * fThickness * 0.1f);
		vec3 f3Refraction = refract(f3ViewDirection, f3Normal, fRadio);
		vec3 f3RefactionColor = texture(uSkyBoxTex, f3Refraction).xyz * f3Transmission;

//Blinn-Phong Shading
		vec4 f4DiffuseLight = calcDirectionalLight(f3WorldPos, f3Normal);

//Color Attenuation from Thickness(Beer's Law)
		float fLightAttenuation = dot(uDirectionalLight.Direction, f3Normal) * 0.05;
		f4DiffuseLight = vec4(mix(f4DiffuseLight.xyz, vec3(1.0f), (fLightAttenuation*0.5 + 0.5) * 0.4f), 1.0f);
		f4Color = f4FluidColor*f4DiffuseLight + vec4(mix(f3RefactionColor, f3ReflectionColor, fFresnel), 1.0f);
		_FluidColor = vec4(f4Color.xyz, 1.0f);
	}

	_FluidColor = vec4(f4Color.xyz, 1.0f);
}