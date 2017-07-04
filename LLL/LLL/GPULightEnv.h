#pragma once
#include <glm/glm.hpp>

struct SGPUBaseLight
{
	glm::vec4 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	glm::vec2 BaseLightAlign;
};

struct SGPUAttenuation
{
	float Constant;
	float Linear;
	float Quadratic;
	float AttenAlign;
};

struct SGPUPointLight
{
	SGPUBaseLight BaseLight;
	glm::vec4 LightPosition;
	SGPUAttenuation Attenuation;
};