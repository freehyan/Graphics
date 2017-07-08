#pragma once
#include <glm/glm.hpp>

struct SLight
{
	glm::vec3 m_LightColor;
	float m_AmbientIntensity;
	float m_DiffuseIntensity;
};

struct SAttenuation
{
	float m_Constant;
	float m_Linear;
	float m_Quadratic;
};

struct SPointLight : public SLight
{
	glm::vec3 m_Position;
	SAttenuation m_Attenuation;
};

struct SSpotLight : public SPointLight
{
	glm::vec3 m_Direction;
	float m_Cutoff;
};