#include "FluidEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>
#include "ThicknessFBO.h"
#include "DepthFBO.h"
#include "CurvatureFlowFBO.h"
#include "BilateralNormalFBO.h"
#include "ShadedNormalFBO.h"
#include "SceneFBO.h"
#include "CompositeFBO.h"

#define FOV 90.0
#define NEAR_P 0.2
#define FAR_P 1000.0
#define CURTURE_FLOW_ITERATION_NUM 10
#define PARTICLE_COUNT 20000

glm::vec3 g_EyeWorldPos = glm::vec3(0.0, 0.0, 20.0);

using namespace Cooler;
Cooler::CProductFactory<CFluidEffect> theCreater("FLUID_EFFECT");
CFluidEffect::CFluidEffect() : m_pInputTransformer(nullptr), m_pThickFBO(nullptr), m_pDepthFBO(nullptr), m_pCurtureFlowFBO(nullptr), m_pBilateralNormalFBO(nullptr),
m_pSceneFBO(nullptr), m_pShadedNormalFBO(nullptr), m_pCompositeFBO(nullptr)
{
	m_ScreenWidth = Cooler::getDisplayWindowsSize().first;
	m_ScreenHeight = Cooler::getDisplayWindowsSize().second;
}

CFluidEffect::~CFluidEffect()
{
}

//***********************************************************
//FUNCTION:
void CFluidEffect::_initEffectV()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	std::vector<glm::vec3> ParticleSet;
	srand(10);
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = 3 + (rand() % 60 + 10.0) / 10.0;
		float y = 3 + (rand() % 40 + 10.0) / 10.0;
		float z = -(rand() % 80 + 10.0) / 10.0;
		ParticleSet.push_back(glm::vec3(x, y, z));
	}
	m_Fluid.initFluidParticles(ParticleSet);

	__initDirectionLight();
	__initFBOBuffers();
	__initTweakBar();
	__createSreenQuad();

	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(0.0f, 0.0f, -20.0f));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	std::vector<std::string> SkyboxFileNameSet;
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/right.jpg");
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/left.jpg");
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/top.jpg");
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/bottom.jpg");
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/back.jpg");
	SkyboxFileNameSet.push_back("../../Resources/Texture/skybox/front.jpg");
	m_CubeMap.initCubeMappingTextureSet(SkyboxFileNameSet);

	m_ProjectionMatrix = glm::perspective(glm::radians(FOV), m_ScreenWidth / (double)m_ScreenHeight, NEAR_P, FAR_P);
}

//***********************************************************
//FUNCTION:
void CFluidEffect::_renderEffectV()
{
	m_ViewMatrix = m_pInputTransformer->getModelViewMat();

	__calcThicknessPass();
	__calcDepthPass();
	__curtureFlowFilterPass();
	__bilateralFilterPass();
	__shadedNormalPass();
	__scenePass();
	__compositePass();
	__showTexturePass(m_pCompositeFBO->m_CompositeTex);
}

//***********************************************************
//FUNCTION:
void CFluidEffect::_destoryEffectV()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__initDirectionLight()
{
	m_DirectionalLight.BaseLight.Color = glm::vec3(1.0, 1.0, 1.0);
	m_DirectionalLight.BaseLight.AmbientIntensity = 0.5;
	m_DirectionalLight.BaseLight.DiffuseIntensity = 0.9;
	m_DirectionalLight.BaseLight.SpecularIntensity = 0.9;
	m_DirectionalLight.BaseLight.SpecularPower = 32.0;
	m_DirectionalLight.Direction = glm::vec3(0.0, 1.0, 0.0);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__initFBOBuffers()
{
	m_pThickFBO = dynamic_cast<CThicknessFBO*>(Cooler::fetchFBO("THICKNESS_FBO"));
	_ASSERTE(m_pThickFBO);

	m_pDepthFBO = dynamic_cast<CDepthFBO*>(Cooler::fetchFBO("DEPTH_FBO"));
	_ASSERTE(m_pDepthFBO);

	m_pCurtureFlowFBO = dynamic_cast<CCurvatureFlowFBO*>(Cooler::fetchFBO("CURVATURE_FLOW_FBO"));
	_ASSERTE(m_pCurtureFlowFBO);

	m_pBilateralNormalFBO = dynamic_cast<CBilateralNormalFBO*>(Cooler::fetchFBO("BILATERAL_NORMAL_FBO"));
	_ASSERTE(m_pBilateralNormalFBO);

	m_pShadedNormalFBO = dynamic_cast<CShadedNormalFBO*>(Cooler::fetchFBO("SHADED_NORMAL_FBO"));
	_ASSERTE(m_pShadedNormalFBO);

	m_pSceneFBO = dynamic_cast<CSceneFBO*>(Cooler::fetchFBO("SCENE_FBO"));
	_ASSERTE(m_pSceneFBO);

	m_pCompositeFBO = dynamic_cast<CCompositeFBO*>(Cooler::fetchFBO("COMPOSITE_FBO"));
	_ASSERTE(m_pCompositeFBO);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__createSreenQuad()
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
void CFluidEffect::__calcThicknessPass()
{
	m_pThickFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 WVPMatrix = m_ProjectionMatrix * m_ViewMatrix * m_WorldMatrix;
	_enableShader("FLUID_THICK_SHADER");

	_updateShaderUniform("uWVPMatrix", WVPMatrix);
	_updateShaderUniform("uWorldMatrix", m_WorldMatrix);
	_updateShaderUniform("uViewMatrix", m_ViewMatrix);
	_updateShaderUniform("uProjectionMatrix", m_ProjectionMatrix);
	_updateShaderUniform("uPointRadius", m_ThicknessScale*m_PointRadius);
//	_updateShaderUniform("uPointScale", (float)m_ScreenHeight*(1.0f / (tanf(FOV * 0.5f))));

	m_Fluid.renderPointSprite2DepthAndColor();

	_disableShader("FLUID_THICK_SHADER");
	m_pThickFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__calcDepthPass()
{
	m_pDepthFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 WVPMatrix = m_ProjectionMatrix * m_ViewMatrix * m_WorldMatrix;
	_enableShader("FLUID_DEPTH_SHADER");
	_updateShaderUniform("uWVPMatrix", WVPMatrix);
	_updateShaderUniform("uWorldMatrix", m_WorldMatrix);
	_updateShaderUniform("uViewMatrix", m_ViewMatrix);
	_updateShaderUniform("uProjectionMatrix", m_ProjectionMatrix);
	_updateShaderUniform("uPointRadius", m_ThicknessScale*m_PointRadius);
//	_updateShaderUniform("uPointScale", (float)m_ScreenHeight*(1.0f / (tanf(FOV * 0.5f))));

	m_Fluid.renderPointSprite2DepthAndColor();

	_disableShader("FLUID_DEPTH_SHADER");
	m_pDepthFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__curtureFlowFilterPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	m_pCurtureFlowFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("FLUID_CURVATURE_FLOW_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pDepthFBO->m_DepthTex);
	_updateShaderUniform("uFluidDepthTex", 0);
	_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));
	_updateShaderUniform("uFOV", (float)FOV);

	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("FLUID_CURVATURE_FLOW_SHADER");
	m_pCurtureFlowFBO->closeFBO();

	_ASSERTE(glGetError() == GL_NO_ERROR);

	for (unsigned int i = 0; i < CURTURE_FLOW_ITERATION_NUM; i++)
	{
		GLuint AuxiliaryTexture = __generateTexture(m_ScreenWidth, m_ScreenHeight);

		GLuint Temp = AuxiliaryTexture;
		AuxiliaryTexture = m_pCurtureFlowFBO->m_SmoothDepthTex;
		m_pCurtureFlowFBO->m_SmoothDepthTex = Temp;

		m_pCurtureFlowFBO->openFBO();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pCurtureFlowFBO->m_SmoothDepthTex, 0);

		_ASSERTE(glGetError() == GL_NO_ERROR);
		glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_enableShader("FLUID_CURVATURE_FLOW_SHADER");

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, AuxiliaryTexture);
		_updateShaderUniform("uFluidDepthTex", 0);
		_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));
		_updateShaderUniform("uFOV", (float)FOV);
		__renderScreenSizeQuad();

		glBindTexture(GL_TEXTURE_2D, 0);
		_disableShader("FLUID_CURVATURE_FLOW_SHADER");
		m_pCurtureFlowFBO->closeFBO();

		glDeleteTextures(1, &AuxiliaryTexture);
	}
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__bilateralFilterPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);

	m_pBilateralNormalFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("FLUID_BILATERAL_FILTER_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pCurtureFlowFBO->m_SmoothDepthTex);
	_updateShaderUniform("uFluidDepthTex", 0);
	_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));
	_updateShaderUniform("uFOV", (float)FOV);

	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("FLUID_BILATERAL_FILTER_SHADER");
	m_pBilateralNormalFBO->closeFBO();

	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__shadedNormalPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	m_pShadedNormalFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("FLUID_SHADED_NORMAL_SHADER");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pDepthFBO->m_DepthTex);
	_updateShaderUniform("uFluidDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pBilateralNormalFBO->m_BilateralNormalTex);
	_updateShaderUniform("uBilateralNormalTex", 1);
	_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));
	_updateShaderUniform("uSampleOffset", (int)4);

	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("FLUID_SHADED_NORMAL_SHADER");
	m_pShadedNormalFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__scenePass()
{
	m_pSceneFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	_enableShader("FLUID_SCENE_SHADER");
	_updateShaderUniform("uViewMatrix", glm::mat4(glm::mat3(m_ViewMatrix)));
	_updateShaderUniform("uProjectionMatrix", m_ProjectionMatrix);

	glActiveTexture(GL_TEXTURE0);
	_updateShaderUniform("uSkyBoxTex", 0);
	m_CubeMap.renderSkybox();

	_disableShader("FLUID_SCENE_SHADER");

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	m_pSceneFBO->closeFBO();
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__compositePass()
{
	m_pCompositeFBO->openFBO();
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("FLUID_COMPOSTE_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pThickFBO->m_ThicknessTex);
	_updateShaderUniform("uThicknessTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pCurtureFlowFBO->m_SmoothDepthTex);
	_updateShaderUniform("uDepthTex", 1);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_pBilateralNormalFBO->m_BilateralNormalTex);
	_updateShaderUniform("uNormalTex", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_pSceneFBO->m_SceneTex);
	_updateShaderUniform("uSceneTex", 3);

	m_CubeMap.bindTexture(GL_TEXTURE4);
	_updateShaderUniform("uSkyBoxTex", 4);

	_updateShaderUniform("uClipPosToEye", glm::vec2(tanf(FOV*0.5f) * ((float)m_ScreenWidth / (float)m_ScreenHeight), tanf(FOV*0.5f)));
	_updateShaderUniform("uEyeWorldPos", g_EyeWorldPos);
	
	glm::mat4 WorldViewInverseMatrix = glm::inverse(m_ViewMatrix * m_WorldMatrix);
	_updateShaderUniform("uWorldViewMatrixInverse", WorldViewInverseMatrix);
	
	_updateShaderUniform("uDirectionalLight.BaseLight.Color", m_DirectinalLight.BaseLight.Color);
	_updateShaderUniform("uDirectionalLight.BaseLight.AmbientIntensity", m_DirectinalLight.BaseLight.AmbientIntensity);
	_updateShaderUniform("uDirectionalLight.BaseLight.DiffuseIntensity", m_DirectinalLight.BaseLight.DiffuseIntensity);
	_updateShaderUniform("uDirectionalLight.BaseLight.SpecularIntensity", m_DirectinalLight.BaseLight.SpecularIntensity);
	_updateShaderUniform("uDirectionalLight.BaseLight.SpecularPower", m_DirectinalLight.BaseLight.SpecularPower);
	_updateShaderUniform("uDirectionalLight.Direction", m_DirectinalLight.Direction);

	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("FLUID_COMPOSTE_SHADER");

	m_pCompositeFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CFluidEffect::__showTexturePass(GLuint vTextureID)
{
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("FLUID_SHOW_TEXTURE_SHADER");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, vTextureID);
	_updateShaderUniform("uColorBuf", 0);
	__renderScreenSizeQuad();

	_disableShader("FLUID_SHOW_TEXTURE_SHADER");
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
GLuint CFluidEffect::__generateTexture(unsigned int vTextureWidth, unsigned int vTextureHeight)
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	GLuint TextureHandle;
	glGenTextures(1, &TextureHandle);
	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, vTextureWidth, vTextureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	_ASSERTE(glGetError() == GL_NO_ERROR);

	return TextureHandle;
}

//***********************************************************
//FUNCTION:
void CFluidEffect::__initTweakBar()
{
	TwBar* pBar = Cooler::fetchTweakBar();
	_ASSERTE(pBar);
}

//***********************************************************
//FUNCTION:
void CFluidEffect::__renderScreenSizeQuad()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}