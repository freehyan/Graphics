#version 450

vec3 LambertDiffuse(vec3 dirToLight, vec3 surfaceNormal, vec3 lightColor, vec3 albedo)
{
	float diffuseAmount = clamp(dot(dirToLight, surfaceNormal), 0.0, 1.0);
	return (diffuseAmount + 0.25f) * lightColor * albedo;
}

vec3 OrenNayar(vec3 dirToLight, vec3 dirToEye, vec3 surfaceNormal, vec3 lightColor, vec3 albedo, float roughness)
{
	float LDotN = dot(dirToLight, surfaceNormal);
	float VDotN = dot(dirToEye, surfaceNormal);

	float thetaI = acos(abs(LDotN));
	float thetaR = acos(abs(VDotN));

	float alpha, beta;
	if (thetaI > thetaR)
	{
		alpha = thetaI; // x
		beta = thetaR;
	}
	else
	{
		alpha = thetaR;
		beta = thetaI;
	}

	vec3 projLight = normalize(dirToLight - (surfaceNormal * clamp(dot(dirToLight, surfaceNormal), 0, 1)));  // dirToLight
	vec3 projEye = normalize(dirToEye - (surfaceNormal * clamp(dot(dirToEye, surfaceNormal), 0, 1)));        // approaches ||0,0,0|| , 

	float gamma = dot(projEye, projLight);
	float roughSqr = roughness * roughness;

	float A = 1.0 - (0.5 * (roughSqr / (roughSqr + 0.57)));
	float B = 0.45 * (roughSqr / (roughSqr + 0.09));
	float C = clamp(sin(alpha), 0, 1) * max(0, tan(beta));

	float final = A + B * max(0, gamma) * C;

	return lightColor * albedo * final * (clamp(LDotN, 0, 1) + 0.25);
}

vec3 DepthToPosition(float depth, vec4 posProj, mat4 projInv, float farClip)
{
	vec3 posView = (projInv * posProj).xyz;
	vec3 viewRay = vec3(posView.xy * (farClip / posView.z), farClip);
	return viewRay * depth;
}