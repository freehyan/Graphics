#pragma once
#include <string>
#include <vector>
#include "LightStructure.h"

class CLightManager
{
public:
	CLightManager(int vLightNum);
	~CLightManager();

	bool genLightSourceSet(int vLightNum, std::vector<SPointLight*>& voLightSourceSet);

	void updateLightSourceSet(std::vector<SPointLight*>& voLightSourceSet, unsigned int vSSBOIndex);

private:
	glm::vec3 __genRandDrection();
	std::vector<glm::vec3> m_LightDeltaDrections;
};