#include "ESMEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>
#include "SrcSMFBO.h"
#include "ESMFBO.h"

#define CAMERA_FOV 60.0f
#define CAMERA_NEAR 1.0f
#define CAMERA_FAR 100.0f
#define ALPHA 80

glm::vec3 g_EyeWorldPos = glm::vec3(0.0, 0.0, 10.0);
glm::vec3 g_LightSourcePos = glm::vec3(5.0, 5.0, 5.0);

using namespace Cooler;
Cooler::CProductFactory<CLLLEffect> theCreater("ESM_EFFECT");
CLLLEffect::CLLLEffect() : m_pInputTransformer(nullptr), m_pSrcSMFBO(nullptr), m_pESMFBO(nullptr)
{
	m_ScreenWidth = Cooler::getDisplayWindowsSize().first;
	m_ScreenHeight = Cooler::getDisplayWindowsSize().second;
}

CLLLEffect::~CLLLEffect()
{
}

//***********************************************************
//FUNCTION:
void CLLLEffect::_initEffectV()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);

	__createSreenQuad();
	__initTweakBar();
	__initSpotLight();

	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(0.0, 0.0, -10.0));
	m_pInputTransformer->setScale(0.1);

	glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	m_pSrcSMFBO = dynamic_cast<CSrcSMFBO*>(Cooler::fetchFBO("SRC_SM_FBO"));
	_ASSERTE(m_pSrcSMFBO);
	m_pESMFBO = dynamic_cast<CESMFBO*>(Cooler::fetchFBO("ESM_FBO"));
	_ASSERTE(m_pESMFBO);

	m_ProjectionMatrix = glm::perspective(glm::radians(CAMERA_FOV), m_ScreenWidth / (float)m_ScreenHeight, CAMERA_NEAR, CAMERA_FAR);
}

//***********************************************************
//FUNCTION:
void CLLLEffect::_renderEffectV()
{
	__SrcShadowMapPass();
	__ExponentialShadowMapPass();
	__LightShadowPass();
}

//***********************************************************
//FUNCTION:
void CLLLEffect::_destoryEffectV()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__createSreenQuad()
{
	GLfloat Vertices[] = {
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__SrcShadowMapPass()
{
	glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
	glm::mat4 LightCameraViewMatrix = glm::lookAt(g_LightSourcePos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	m_pSrcSMFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MVPMatrix = m_ProjectionMatrix * LightCameraViewMatrix * ModelMatrix;
	m_LightSpaceMVPMatrix = MVPMatrix;

	_enableShader("ESM_SHADOW_MAP_SHADER");
	_updateShaderUniform("uMVPMatrix", MVPMatrix);
	_updateShaderUniform("uAlpha", ALPHA);

	Cooler::graphicsRenderModel("PALM");

	_disableShader("ESM_SHADOW_MAP_SHADER");
	m_pSrcSMFBO->closeFBO();
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__ExponentialShadowMapPass()
{
	m_pESMFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("ESM_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pSrcSMFBO->m_ESMTex);
	_updateShaderUniform("uShadowMapTexture", 0);

	_updateShaderUniform("uTextureWidth", (int)m_ScreenWidth);
	_updateShaderUniform("uTextureHeight", (int)m_ScreenHeight);

	__renderScreenSizeQuad();
	_disableShader("ESM_SHADER");
	m_pESMFBO->closeFBO();
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__LightShadowPass()
{
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
	glm::mat4 MainCameraViewMatrix = m_pInputTransformer->getModelViewMat();
	glm::mat4 MVPMatrix = m_ProjectionMatrix * MainCameraViewMatrix;

	_enableShader("ESM_LIGHT_SHADOW_SHADER");
	_updateShaderUniform("uModelMatrix", ModelMatrix);
	_updateShaderUniform("uMVPMatrix", MVPMatrix);
	_updateShaderUniform("uLightSpaceMVPMatrix", m_LightSpaceMVPMatrix);	

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pESMFBO->m_PrefilterTexZ);
	_updateShaderUniform("uPrefilterTextureZ", 1);
	_updateShaderUniform("uEyeWorldPos", g_EyeWorldPos);
	_updateShaderUniform("uAlpha", ALPHA);

	__setSpotLight(1, m_pSpotLight);

	Cooler::graphicsRenderModel("PALM");
	_disableShader("ESM_LIGHT_SHADOW_SHADER");
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__initSpotLight()
{
	m_pSpotLight = new SSpotLight;
	m_pSpotLight->m_LightColor = glm::vec3(1.0, 1.0, 1.0);
	m_pSpotLight->m_AmbientIntensity = 0.1f;
	m_pSpotLight->m_DiffuseIntensity = 0.9f;
	m_pSpotLight->m_Position = g_LightSourcePos;
	m_pSpotLight->m_Direction = -g_LightSourcePos;

	m_pSpotLight->m_Cutoff = glm::cos(glm::radians(CAMERA_FOV));
	m_pSpotLight->m_Attenuation.m_Constant = 1.0f;
	m_pSpotLight->m_Attenuation.m_Linear = 0.0f;
	m_pSpotLight->m_Attenuation.m_Quadratic = 0.0f;
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__setSpotLight(unsigned int vLightNum, const SSpotLight* vSpotLight)
{
	_updateShaderUniform("uSpotLightNum", vLightNum);

	for (unsigned int i = 0; i < vLightNum; i++)
	{
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.BaseLight.Color", vSpotLight[i].m_LightColor);
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.BaseLight.AmbientIntensity", vSpotLight[i].m_AmbientIntensity);
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.BaseLight.DiffuseIntensity", vSpotLight[i].m_DiffuseIntensity);

		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.LightPosition", vSpotLight[i].m_Position);

		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].Direction", glm::normalize(vSpotLight[i].m_Direction));
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].Cutoff", vSpotLight[i].m_Cutoff);

		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.Attenuation.Constant", vSpotLight[i].m_Attenuation.m_Constant);
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.Attenuation.Linear", vSpotLight[i].m_Attenuation.m_Linear);
		_updateShaderUniform("uSpotLights[" + std::to_string(i) + "].PointLight.Attenuation.Quadratic", vSpotLight[i].m_Attenuation.m_Quadratic);
	}
}

//***********************************************************
//FUNCTION:
void CLLLEffect::__initTweakBar()
{
	TwBar* pBar = Cooler::fetchTweakBar();
	_ASSERTE(pBar);
}

//***********************************************************
//FUNCTION:
void CLLLEffect::__renderScreenSizeQuad()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}