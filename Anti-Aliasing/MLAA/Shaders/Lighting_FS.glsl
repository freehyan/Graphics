#version 450

in vec2 _TexCoord;

uniform sampler2D uNormalTex;

layout(location = 0) out vec4 FS_LightingColor;

vec3 LambertDiffuse(vec3 dirToLight, vec3 surfaceNormal, vec3 lightColor, vec3 albedo)
{
	float diffuseAmount = clamp(dot(dirToLight, surfaceNormal), 0.0, 1.0);
	return (diffuseAmount + 0.25f) * lightColor * albedo;
}


void main()
{	
	vec3 LightSourceColor =  vec3(1.0, 1.0, 1.0);
	vec3 DiffuseColor = vec3(1.0, 1.0, 1.0);
	vec3 DirToLight = normalize(vec3(-1, 1, -1));

	vec3 WorldNormal = normalize(texture2D(uNormalTex, _TexCoord).xyz);
	vec3 LightingColor = LambertDiffuse(DirToLight, WorldNormal, LightSourceColor, DiffuseColor);

	FS_LightingColor = vec4(LightingColor, 1.0);
}