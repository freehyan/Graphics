#include "LightManager.h"
#include "BoxManager.h"
#include "GPULightEnv.h"
#include "AABB.h"

CLightManager::CLightManager(int vLightNum)
{
	m_LightDeltaDrections.resize(vLightNum);
	for (auto Drection : m_LightDeltaDrections)
	{
		Drection = __genRandDrection();
	}
}

CLightManager::~CLightManager()
{
}

//***********************************************************
//FUNCTION:
bool CLightManager::genLightSourceSet(int vLightNum, std::vector<GraphicsCommon::CPointLight*>& voLightSourceSet)
{
	int i = 0;
	while (i < vLightNum)
	{
		glm::vec3 Max, Min;
		(dynamic_cast<CAABB*>(CBoxManager::getInstance()->m_Boxset[0]))->dumpPointMax(Max);
		(dynamic_cast<CAABB*>(CBoxManager::getInstance()->m_Boxset[0]))->dumpPointMin(Min);

		float X = rand() % (((Max.x - Min.x)>1.0) ? (int)(Max.x - Min.x) : 1) + Min.x;
		float Y = rand() % (((Max.y - Min.y)>1.0) ? (int)(Max.y - Min.y) : 1) + Min.y;
		float Z = rand() % (((Max.z - Min.z)>1.0) ? (int)(Max.z - Min.z) : 1) + Min.z;
		glm::vec3 Pos = glm::vec3(X, Y, Z);

		if (CBoxManager::getInstance()->isPointPositionValid(Pos))
		{
			float Red = float(rand() % 10) / 10.0f;
			float Green = float(rand() % 10) / 10.0f;
			float Blue = float(rand() % 10) / 10.0f;

			GraphicsCommon::CPointLight* pPointLight = new GraphicsCommon::CPointLight;
			pPointLight->m_LightColor = glm::vec3(Red, Green, Blue);
			pPointLight->m_AmbientIntensity = 0.2;
			pPointLight->m_DiffuseIntensity = 0.9;
			pPointLight->m_Position = Pos;
			pPointLight->m_Attenuation.m_Constant = 1.0f;
			pPointLight->m_Attenuation.m_Linear = 0.1f;
			pPointLight->m_Attenuation.m_Quadratic = 0.0f;
			voLightSourceSet.push_back(pPointLight);
			i++;
		}
	}

	return true;
}

//***********************************************************
//FUNCTION:
void CLightManager::updateLightSourceSet(std::vector<GraphicsCommon::CPointLight*>& voLightSourceSet, unsigned int vSSBOIndex)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vSSBOIndex);
	SPointLight* pGPUSSBOData = (SPointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	_ASSERT(pGPUSSBOData);

	unsigned int i = 0;
	for (auto Light : voLightSourceSet)
	{
		if ((CBoxManager::getInstance()->isPointPositionValid(Light->m_Position + m_LightDeltaDrections[i])) && ((m_LightDeltaDrections[i].x != 0) || (m_LightDeltaDrections[i].y != 0) || (m_LightDeltaDrections[i].z != 0)))
		{
			Light->m_Position += m_LightDeltaDrections[i];
		}
		else
		{
			m_LightDeltaDrections[i] = __genRandDrection();
		}

		pGPUSSBOData[i].LightPosition = glm::vec4(Light->m_Position, 0.0);
		i++;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

//***********************************************************
//FUNCTION:
glm::vec3 CLightManager::__genRandDrection()
{
	float DeltaX = (float(rand() % 3) - 1) / 1;
	float DeltaY = (float(rand() % 3) - 1) / 1;
	float DeltaZ = (float(rand() % 3) - 1) / 1;
	glm::vec3 DeltaMove = glm::vec3(DeltaX, DeltaY, DeltaZ);

	return DeltaMove;
}