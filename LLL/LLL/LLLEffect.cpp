#include "LLLEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>
#include "DeferredFBO.h"

// #define SCREEN_WIDTH 1024
// #define SCREEN_HEIGHT 768
// #define LLL_WIDTH 512
// #define LLL_HEIGHT 384
// #define SCREENLLLRATIO SCREEN_WIDTH/LLL_WIDTH
#define POINT_LIGHT_RADIUS 10

#define POINT_LIHGT_NUM 100
#define MAX_BOUNDARY 10
#define MIN_BOUNDARY -10
#define MATH_PI 3.141592653589793
#define MODEL_SCALE 0.1f

glm::vec3 g_EyeWorldPos = glm::vec3(0.0, 0.0, 15.0);

using namespace Cooler;
Cooler::CProductFactory<CLLLEffect> theCreater("LLL_EFFECT");
CLLLEffect::CLLLEffect() : m_pInputTransformer(nullptr), m_pDeferredFBO(nullptr)
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
	__initDirectionLight();
	__initTransparentMeshPosition();
	__initLLLUAVBuffers();

	__createSreenQuad();
	__createLightSSBO();

	__initTweakBar();

	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(0.0, 0.0, -15.0));
//	m_pInputTransformer->setRotationVec(glm::vec3(0.0, 90.0, 0.0));
	m_pInputTransformer->setScale(0.1f);
	m_pInputTransformer->setMotionMode(Cooler::SCameraMotionType::FIRST_PERSON);

	//glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	m_pDeferredFBO = dynamic_cast<CDeferredFBO*>(Cooler::fetchFBO("DEFERRED_FBO"));
	_ASSERTE(m_pDeferredFBO);
	m_ProjectionMatrix = glm::perspective(glm::radians(60.0f), m_ScreenWidth / (float)m_ScreenHeight, 0.1f, 1000.0f);

	glEnable(GL_DEPTH_TEST);
}

//***********************************************************
//FUNCTION:
void CLLLEffect::_renderEffectV()
{
	m_pDeferredFBO->openFBO();
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	glClear(GL_COLOR_BUFFER_BIT);

	__DSGemetroyPass();
	__createLLLCullingBackFacePass();
	__createLLLCullingFrontFacePass();

	__renderOpaqueScenePass();
	__DSDirectionalLightingPass();

	__renderTransparentScenePass();

// 	if (m_bMoveLightSource)
// 		__updateLightSourceSetPosition();

	m_pDeferredFBO->closeFBO();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pDeferredFBO->getFBOId());
	glReadBuffer(GL_COLOR_ATTACHMENT3);
	glBlitFramebuffer(0, 0, m_ScreenWidth, m_ScreenHeight, 0, 0, m_ScreenWidth, m_ScreenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
void CLLLEffect::__initDirectionLight()
{
	m_DirectinalLight.m_LightColor = glm::vec3(1.0, 1.0, 1.0);
	m_DirectinalLight.m_AmbientIntensity = 0.2f;
	m_DirectinalLight.m_DiffuseIntensity = 0.5f;
	m_DirectinalLight.m_Direction = glm::vec3(-1.0f, -1.0f, -1.0f);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__initTransparentMeshPosition()
{
	m_BottlePosition.push_back(glm::vec3(10.0, 0.0, -10.0));
	m_BottlePosition.push_back(glm::vec3(-10.0, 0.0, 10.0));
	m_BottlePosition.push_back(glm::vec3(-10.0, 0.0, -10.0));
	m_BottlePosition.push_back(glm::vec3(10.0, 0.0, 10.0));
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__initLLLUAVBuffers()
{
	GLuint* pData = nullptr;
	unsigned int ScreenTotalPixels = (unsigned int)m_ScreenWidth * m_ScreenHeight;

	//生成Light start offset texture
	glGenTextures(1, &m_LightStartOffsetTex);
	glBindTexture(GL_TEXTURE_2D, m_LightStartOffsetTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_ScreenWidth, m_ScreenHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//每帧都需要重初始化StartOffsetBuffer,使用PBO实现
	glGenBuffers(1, &m_LightStartOffsetBufferPBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_LightStartOffsetBufferPBO);//GL_PIXEL_UNPACK_BUFFER通常作为glTexImage2D等纹理命令数据来源
	glBufferData(GL_PIXEL_UNPACK_BUFFER, ScreenTotalPixels * sizeof(GLuint), NULL, GL_STATIC_DRAW);

	pData = (GLuint*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	memset(pData, 0x00, ScreenTotalPixels * sizeof(GLuint));
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	//创建原子计数器缓存
	glGenBuffers(1, &m_AtomicCounterBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_AtomicCounterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	glGenBuffers(1, &m_LightFragmentLinkedBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, m_LightFragmentLinkedBuffer);
	glBufferData(GL_TEXTURE_BUFFER, POINT_LIHGT_NUM*ScreenTotalPixels * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glGenTextures(1, &m_LightFragmentLinkedTexture);	//创建TBO
	glBindTexture(GL_TEXTURE_BUFFER, m_LightFragmentLinkedTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, m_LightFragmentLinkedBuffer); //绑定到缓存和设定格式
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__initLightSourcesData()
{
	for (int i = 0; i < POINT_LIHGT_NUM; i++)
	{
		SPointLight* pPointLight = new SPointLight;
		float Red = rand() % 10 / 10.0;
		float Green = rand() % 10 / 10.0;
		float Blue = rand() % 10 / 10.0;

		float X = rand() % 20 - 10;	//X 范围 
		float Y = rand() % 15;
		float Z = rand() % 20 - 10;

		pPointLight->m_LightColor = glm::vec3(Red, Green, Blue);
		pPointLight->m_AmbientIntensity = 0.2;
		pPointLight->m_DiffuseIntensity = 1.0;
		pPointLight->m_Position = glm::vec3(X, Y, Z);
		pPointLight->m_Attenuation.m_Constant = 1.0f;
		pPointLight->m_Attenuation.m_Linear = 0.0f;
		pPointLight->m_Attenuation.m_Quadratic = 0.0f;
		m_PointLightSourceSet.push_back(pPointLight);

		glm::vec3 Pos = glm::vec3(0.0, m_PointLightSourceSet[i]->m_Position.y, 0.0);
		float Radius = sqrtf(glm::dot(Pos - m_PointLightSourceSet[i]->m_Position, Pos - m_PointLightSourceSet[i]->m_Position));
		float CosTheta = m_PointLightSourceSet[i]->m_Position.x / Radius;
		float RadiansDegree = glm::acos(CosTheta);
		float Degree = RadiansDegree * 180 / MATH_PI;
		m_InitDegree.push_back(Degree);
	}
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
void CLLLEffect::__createLightSSBO()
{
	unsigned int LightCount = m_PointLightSourceSet.size();
	unsigned int StructSize = sizeof(SGPUPointLight);

	SGPUPointLight* pPointLightSet = new SGPUPointLight[m_PointLightSourceSet.size()];
	memset(pPointLightSet, 0, sizeof(SGPUPointLight)*m_PointLightSourceSet.size());

	for (unsigned int i = 0; i < m_PointLightSourceSet.size(); i++)
	{
		pPointLightSet[i].BaseLight.Color = glm::vec4(m_PointLightSourceSet[i]->m_LightColor, 0.0);
		pPointLightSet[i].BaseLight.AmbientIntensity = m_PointLightSourceSet[i]->m_AmbientIntensity;
		pPointLightSet[i].BaseLight.DiffuseIntensity = m_PointLightSourceSet[i]->m_DiffuseIntensity;
		pPointLightSet[i].BaseLight.BaseLightAlign = glm::vec2(0.0, 0.0);

		//Light Position 
		pPointLightSet[i].LightPosition = glm::vec4(m_PointLightSourceSet[i]->m_Position, 0.0);

		//Attenuation
		pPointLightSet[i].Attenuation.Constant = m_PointLightSourceSet[i]->m_Attenuation.m_Constant;
		pPointLightSet[i].Attenuation.Linear = m_PointLightSourceSet[i]->m_Attenuation.m_Linear;
		pPointLightSet[i].Attenuation.Quadratic = m_PointLightSourceSet[i]->m_Attenuation.m_Quadratic;
		pPointLightSet[i].Attenuation.AttenAlign = 0.0;
	}

	m_SSBOBlockIndex = glGetProgramResourceIndex(_getShaderId("LLL_RENDER_OPAQUE_SCENE"), GL_SHADER_STORAGE_BLOCK, "ssbo_block_GPULightEnv");
	GLint SSBOBinding = 0, BlockDataSize = 0;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &SSBOBinding);
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &BlockDataSize);

	glGenBuffers(1, &m_LightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LightCount * sizeof(SGPUPointLight), pPointLightSet, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	delete[]pPointLightSet;
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
void CLLLEffect::__DSGemetroyPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);

	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(0.1f));;
	glm::mat4 ViewMatrix = m_pInputTransformer->getModelViewMat();

	glm::mat4 MVPMatrix = m_ProjectionMatrix * ViewMatrix * ModelMatrix;
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));

	_enableShader("LLL_GEOMETRY_SHADER");

	_updateShaderUniform("_uMVPMatrix", MVPMatrix);
	_updateShaderUniform("_uModelMatrix", ModelMatrix);
	_updateShaderUniform("_uNormalMatrix", NormalMatrix);

	Cooler::graphicsRenderModel("SPONZA");
	_disableShader("LLL_GEOMETRY_SHADER");

	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__createLLLCullingBackFacePass()
{
	glDrawBuffer(GL_NONE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	__resetLightStartOffsetBuffer();

	_enableShader("LLL_DEPTH_BOUND_CULL_BACK_FACE");
	glBindImageTexture(0, m_LightStartOffsetTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, m_LightFragmentLinkedTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);

	glm::mat4 ViewMatrix = m_pInputTransformer->getModelViewMat();

	for (unsigned int i = 0; i < POINT_LIHGT_NUM; i++)
	{
		float BSphereScale = __calcPointLightBSphere(*m_PointLightSourceSet[i]);
		glm::mat4 Model = glm::scale(glm::mat4(1.0), glm::vec3(BSphereScale, BSphereScale, BSphereScale));
		Model = glm::translate(Model, m_PointLightSourceSet[i]->m_Position);
		glm::mat4 MVPMatrix = m_ProjectionMatrix * ViewMatrix * Model;
		
		_updateShaderUniform("uMVPMatrix", MVPMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->getFBODepth());
		
		_updateShaderUniform("uDepthTex", 0);
		_updateShaderUniform("uLightIndex", i);
		_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));
		Cooler::graphicsRenderModel("SPHERE");
	}

	_disableShader("LLL_DEPTH_BOUND_CULL_BACK_FACE");
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_CULL_FACE);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__createLLLCullingFrontFacePass()
{
	glDrawBuffer(GL_NONE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	_enableShader("LLL_DEPTH_BOUND_CULL_FRONT_FACE");
	glBindImageTexture(0, m_LightStartOffsetTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	glBindImageTexture(1, m_LightFragmentLinkedTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);

	glm::mat4 ViewMatrix = m_pInputTransformer->getModelViewMat();
	for (unsigned int i = 0; i < POINT_LIHGT_NUM; i++)
	{
		float BSphereScale = __calcPointLightBSphere(*m_PointLightSourceSet[i]);
		glm::mat4 Model = glm::scale(glm::mat4(1.0), glm::vec3(BSphereScale, BSphereScale, BSphereScale));
		Model = glm::translate(Model, m_PointLightSourceSet[i]->m_Position);
		glm::mat4 MVPMatrix = m_ProjectionMatrix * ViewMatrix * Model;
		_updateShaderUniform("uMVPMatrix", MVPMatrix);
		_updateShaderUniform("uLightIndex", i);
		Cooler::graphicsRenderModel("SPHERE");
	}

	_enableShader("LLL_DEPTH_BOUND_CULL_FRONT_FACE");
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_CULL_FACE);
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__renderOpaqueScenePass()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	glDisable(GL_DEPTH_TEST);

	_enableShader("LLL_RENDER_OPAQUE_SCENE");

	GLuint BindingPointIndex = 1;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BindingPointIndex, m_LightSSBO);
	glShaderStorageBlockBinding(_getShaderId("LLL_RENDER_OPAQUE_SCENE"), m_SSBOBlockIndex, BindingPointIndex);

	_updateShaderUniform("uEyeWorldPos", g_EyeWorldPos);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_PosTex);
	_updateShaderUniform("uWorldPosTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->getFBODepth());
	_updateShaderUniform("uDepthTex", 1);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_NormalTex);
	_updateShaderUniform("uNormalTex", 2);
	_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));


	__renderScreenSizeQuad();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("LLL_RENDER_OPAQUE_SCENE");
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__DSDirectionalLightingPass()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT3);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	_enableShader("LLL_DIRECTIONAL_LIGHT");
	glm::mat4 MVPMatrix = glm::mat4(1.0);
	_updateShaderUniform("_uMVPMatrix", MVPMatrix);

	__setDirectionalLight();
	_updateShaderUniform("uEyeWorldPos", g_EyeWorldPos);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_PosTex);
	_updateShaderUniform("uWorldPosTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_ColorTex);
	_updateShaderUniform("uColorTex", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_NormalTex);
	_updateShaderUniform("uNormalTex", 2);
	_updateShaderUniform("uScreenSize", glm::vec2(m_ScreenWidth, m_ScreenHeight));

	__renderScreenSizeQuad();
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("LLL_DIRECTIONAL_LIGHT");
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__renderTransparentScenePass()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//打开混合，透明物体和已渲染的不透明物体进行默认混合
	glEnable(GL_BLEND);
	// 	glBlendEquation(GL_FUNC_ADD);
	// 	glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 ViewMatrix = m_pInputTransformer->getModelViewMat();
	glm::mat4 MVPMatrix, NormalMatrix;

	_enableShader("LLL_RENDER_TRANSPARENT");
	GLuint BindingPointIndex = 1;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BindingPointIndex, m_LightSSBO);
	glShaderStorageBlockBinding(_getShaderId("LLL_RENDER_TRANSPARENT"), m_SSBOBlockIndex, BindingPointIndex);

	__setDirectionalLight();
	_updateShaderUniform("uColorTex", 0);
	_updateShaderUniform("uEyeWorldPos", g_EyeWorldPos);

	glm::mat4 Model;
	for (unsigned int i = 0; i < m_BottlePosition.size(); i++)
	{
		Model = glm::mat4(1.0);
// 		Model = glm::rotate(Model, glm::radians((GLfloat)m_MotionX), glm::vec3(1.0, 0.0, 0.0));
// 		Model = glm::rotate(Model, glm::radians((GLfloat)m_MotionY), glm::vec3(0.0, 1.0, 0.0));
		Model = glm::translate(Model, m_BottlePosition[i]);
		MVPMatrix = m_ProjectionMatrix * ViewMatrix * Model;
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * Model));

		_updateShaderUniform("uMVPMatrix", MVPMatrix);
		_updateShaderUniform("uModelMatrix", Model);
		_updateShaderUniform("uNormalMatrix", NormalMatrix);
		Cooler::graphicsRenderModel("BOTTLE");
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("LLL_RENDER_TRANSPARENT");
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__resetLightStartOffsetBuffer()
{
	GLuint* pData = nullptr;
	//重置atomic counter
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_AtomicCounterBuffer);
	pData = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
	pData[0] = 0;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	//清除start offset buffer image
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_LightStartOffsetBufferPBO);
	glBindTexture(GL_TEXTURE_2D, m_LightStartOffsetTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_ScreenWidth, m_ScreenHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

//***********************************************************
//FUNCTION::
float CLLLEffect::__calcPointLightBSphere(const SPointLight& vLight)
{
	GLfloat LightMax = FMAX(FMAX(vLight.m_LightColor.x, vLight.m_LightColor.y), vLight.m_LightColor.z);

	float Delta = sqrtf(vLight.m_Attenuation.m_Linear*vLight.m_Attenuation.m_Linear - 4 * vLight.m_Attenuation.m_Quadratic*(vLight.m_Attenuation.m_Constant - (256.0 / 5.0) * LightMax));
	float Radius = (-vLight.m_Attenuation.m_Linear + Delta) / (2 * vLight.m_Attenuation.m_Quadratic);
	Radius = 10;
	_ASSERT(Radius > 0.0);

	return Radius;
}

//***********************************************************
//FUNCTION::
void CLLLEffect::__setDirectionalLight()
{
	_updateShaderUniform("uDirectionalLight.BaseLight.Color", glm::vec4(m_DirectinalLight.m_LightColor, 0.0));
	_updateShaderUniform("uDirectionalLight.BaseLight.AmbientIntensity", m_DirectinalLight.m_AmbientIntensity);
	_updateShaderUniform("uDirectionalLight.BaseLight.DiffuseIntensity", m_DirectinalLight.m_DiffuseIntensity);
	_updateShaderUniform("uDirectionalLight.Direction", glm::vec4(m_DirectinalLight.m_Direction, 0.0));

}

//***********************************************************
//FUNCTION:
void CLLLEffect::__renderScreenSizeQuad()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}