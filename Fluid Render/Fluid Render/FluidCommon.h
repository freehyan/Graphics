#pragma once
#include <glm/glm.hpp>

struct SParticle
{
	glm::vec3 mPos;
};

struct SBaseLight
{
	glm::vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	float SpecularIntensity;
	float SpecularPower;
};

struct SDirectionalLight
{
	SBaseLight BaseLight;
	glm::vec3 Direction;
};