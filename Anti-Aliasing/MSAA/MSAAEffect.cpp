#include "MSAAEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>

Cooler::CProductFactory<CMSAAEffect> theCreater("MSAA_EFFECT");

CMSAAEffect::CMSAAEffect() : m_pInputTransformer(nullptr), m_pMSAAFBO(nullptr), m_BRDF(0)
{
}

CMSAAEffect::~CMSAAEffect()
{
	delete m_pMSAAFBO;
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::_initEffectV()
{
	__initTweakBar();
	_ASSERTE(glGetError() == GL_NO_ERROR);
	__initSceneData();
	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(100.0f, -80.0f, 0.0f));
	m_pInputTransformer->setRotationVec(glm::vec3(0.0, 90.0, 0.0));
	m_pInputTransformer->setScale(0.1f);
	m_pInputTransformer->setMotionMode(Cooler::SCameraMotionType::FIRST_PERSON);

	glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	m_pMSAAFBO = new CMSAAFBO;
	m_pMSAAFBO->initRenderTarget_PerPixelShading();
	m_ProjectionMatrix = glm::perspective(3.14f * 0.25f, 1280 / (float)720, 0.1f, 1000.0f);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CMSAAEffect::_renderEffectV()
{
	__updateRenderTargets();

	glClearDepth(1.0f);
	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_ModelViewMatrix = m_pInputTransformer->getModelViewMat();
	m_NormalMatrix = m_pInputTransformer->getRotationMat();

	_ASSERTE(glGetError() == GL_NO_ERROR);
	if (m_MSAAMehod == 0)
		__geometryPass_NoMSAA();
	
	else
		__geometryPass();
	
	if (m_MSAAMehod == 1)
	{
		__resolveMSAAGBuffer();
	}
	
	if (m_MSAAMehod == 0)
	{
		__lightingPass();
	}
	else if(m_bSeparateComplexPass)
	{
		__fillStencilBuffer();
		if (m_bUsePerSamplePixelShader)
			__performDualPassLighting_PerSample();
		else
			__performDualPassLighting_PerPixel();

	}
	else
		__performSingelPassLighting();

	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CMSAAEffect::__geometryPass_NoMSAA()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);

	glBindFramebuffer(GL_FRAMEBUFFER, m_pMSAAFBO->m_GBufferFboId);
	const GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers);

	float ClearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
	glClearBufferfv(GL_COLOR, 0, ClearColorZero);
	glClearBufferfv(GL_COLOR, 1, ClearColorZero);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	glm::mat4 MVPMatrix = m_ProjectionMatrix * m_ModelViewMatrix;

	_enableShader("NO_MSAA_GEOMETRY_SHADER");

	_updateShaderUniform("uModelViewMatrix", m_ModelViewMatrix);
	_updateShaderUniform("uModelViewProjMatrix", MVPMatrix);

	Cooler::graphicsRenderModel("SPONZA");

	_disableShader("NO_MSAA_GEOMETRY_SHADER");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__lightingPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	_enableShader("NO_MSAA_LIGHTING_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_pMSAAFBO->m_GBufferTex[0]);
	_updateShaderUniform("uNormalDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_pMSAAFBO->m_GBufferTex[1]);
	_updateShaderUniform("uDiffuseColorTex", 1);

	_updateShaderUniform("uProjViewMatrix", glm::inverse(m_ProjectionMatrix));
	_updateShaderUniform("uViewWorldMatrix", glm::inverse(m_ModelViewMatrix));

	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	_disableShader("NO_MSAA_LIGHTING_SHADER");
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__initTweakBar()
{
	TwBar* pBar = Cooler::fetchTweakBar();
	_ASSERTE(pBar);

	TwAddVarCB(pBar, "Method", TW_TYPE_INT32, setMSAAMethod, getMSAAMethod, NULL, " min=0 max=2 ");
	TwAddVarRW(pBar, "Mark", TW_TYPE_BOOL32, &m_bMarkComplexPixels, " label='Mark Complex Pixels'");

	TwAddVarCB(pBar, "Separate", TW_TYPE_BOOL32, setSeparateComplexPassCB, getSeparateComplexPassCB, NULL, " label='Separate Complex Pass' ");
	TwAddVarCB(pBar, "Adaptive", TW_TYPE_BOOL32, setAdaptiveShadingCB, getAdaptiveShadingCB, NULL, " label='Adaptively Shade Complex Pixels' ");
	TwAddVarCB(pBar, "Per Sample", TW_TYPE_BOOL32, setPerSampleCB, getPerSampleCB, NULL, "label = 'Per Sample Pixel Shader'");
	TwAddVarRW(pBar, "BRDF", TW_TYPE_BOOL32, &m_BRDF, " label='BRDF Lighting Model'");
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__geometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_pMSAAFBO->m_GBufferFboId);

	const GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);

	float ClearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
	glClearBufferfv(GL_COLOR, 0, ClearColorZero);
	glClearBufferfv(GL_COLOR, 1, ClearColorZero);
	glClearBufferfv(GL_COLOR, 2, ClearColorZero);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	glm::mat4 MVPMatrix = m_ProjectionMatrix * m_ModelViewMatrix;
	_enableShader("MSAA_GEOMETRY_SHADER");
	_updateShaderUniform("uModelViewMatrix", m_ModelViewMatrix);
	_updateShaderUniform("uModelViewProjMatrix", MVPMatrix);

	_updateShaderUniform("uSampleMask", (int)((1 << m_pMSAAFBO->m_iMSAACount) - 1));

	Cooler::graphicsRenderModel("SPONZA");
	_disableShader("MSAA_GEOMETRY_SHADER");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__resolveMSAAGBuffer()
{
	// Do a standard GL resolve of an MSAA surface to a non-MSAA surface by blitting from one to the other
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pMSAAFBO->m_GBufferFboId);
	glReadBuffer(GL_COLOR_ATTACHMENT1);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pMSAAFBO->m_ResolveFboId);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glBlitFramebuffer(0, 0, m_pMSAAFBO->m_ImageWidth, m_pMSAAFBO->m_ImageHeight, 0, 0, m_pMSAAFBO->m_ImageWidth, m_pMSAAFBO->m_ImageHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__fillStencilBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_pMSAAFBO->m_LightingAccumulationBufferFboId);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	glClear(GL_STENCIL_BUFFER_BIT);

	_enableShader("MASK_GENERATION_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[0]);
	_updateShaderUniform("uNormalDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[1]);
	_updateShaderUniform("uDiffuseEdgeTex", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(0x84F5/*GL_TEXTURE_RECT*/, m_pMSAAFBO->m_ResolveColorTex);
	_updateShaderUniform("uResolvedColorTex", 2);

	_updateShaderUniform("uUseDiscontinuity", m_MSAAMehod==EMSAAMethod::APPROACH_DISCONTINUITY);

	__renderScreenSizeQuad();

	_disableShader("MASK_GENERATION_SHADER");
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__performSingelPassLighting()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	_enableShader("MSAA_LIGHTING_SHADER");
	
	_updateShaderUniform("uProjViewMatrix", glm::inverse(m_ProjectionMatrix));
	_updateShaderUniform("uViewWorldMatrix", glm::inverse(m_pInputTransformer->getModelViewMat()));

	// Pass all of the parameters that define our lighting and MSAA mode to the shader
	_updateShaderUniform("uSeparateEdgePass", m_bSeparateComplexPass);
	_updateShaderUniform("uSecondPass", false);
	_updateShaderUniform("uUseDiscontinuity", m_MSAAMehod == EMSAAMethod::APPROACH_DISCONTINUITY);
	_updateShaderUniform("uAdaptiveShading", m_bAdaptiveShading);
	_updateShaderUniform("uShowComplexPixels", m_bMarkComplexPixels);
	_updateShaderUniform("uLightingModel", m_BRDF);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[0]);
	_updateShaderUniform("uNormalDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[1]);
	_updateShaderUniform("uDiffuseEdgeTex", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[2]);
	_updateShaderUniform("uCoverageTex", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_pMSAAFBO->m_ResolveColorTex);
	_updateShaderUniform("uResolvedColorTex", 3);

	__renderScreenSizeQuad();

	_disableShader("MSAA_LIGHTING_SHADER");
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__performDualPassLighting_PerPixel()
{
	glDisable(GL_DEPTH_TEST);

	_enableShader("MSAA_LIGHTING_SHADER");

	_updateShaderUniform("uProjViewMatrix", glm::inverse(m_ProjectionMatrix));
	_updateShaderUniform("uViewWorldMatrix", glm::inverse(m_pInputTransformer->getModelViewMat()));

	// Pass all of the parameters that define our lighting and MSAA mode to the shader
	_updateShaderUniform("uSeparateEdgePass", m_bSeparateComplexPass);
	_updateShaderUniform("uUseDiscontinuity", m_MSAAMehod == EMSAAMethod::APPROACH_DISCONTINUITY);
	_updateShaderUniform("uAdaptiveShading", m_bAdaptiveShading);
	_updateShaderUniform("uShowComplexPixels", m_bMarkComplexPixels);
	_updateShaderUniform("uLightingModel", m_BRDF);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[0]);
	_updateShaderUniform("uNormalDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[1]);
	_updateShaderUniform("uDiffuseEdgeTex", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[2]);
	_updateShaderUniform("uCoverageTex", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(0x84F5/*GL_TEXTURE_RECT*/, m_pMSAAFBO->m_ResolveColorTex);
	_updateShaderUniform("uResolvedColorTex", 3);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0x00); // Disable writing to the stencil mask

	// Render First (Simple Pixel) Pass
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	_updateShaderUniform("uSecondPass", false);
	__renderScreenSizeQuad();

	// Render Second (Complex Pixel) Pass
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	_updateShaderUniform("uSecondPass", true);
	__renderScreenSizeQuad();

	glDisable(GL_STENCIL_TEST);
	_disableShader("MSAA_LIGHTING_SHADER");
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__performDualPassLighting_PerSample()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_pMSAAFBO->m_OffScreenFboId);

	glDisable(GL_DEPTH_TEST);

	_enableShader("MSAA_LIGHTING_SHADER");
	_updateShaderUniform("uProjViewMatrix", glm::inverse(m_ProjectionMatrix));
	_updateShaderUniform("uViewWorldMatrix", glm::inverse(m_pInputTransformer->getModelViewMat()));


	// Pass all of the parameters that define our lighting and MSAA mode to the shader
	_updateShaderUniform("uSeparateEdgePass", m_bSeparateComplexPass);
	_updateShaderUniform("uSecondPass", false);
	_updateShaderUniform("uUseDiscontinuity", m_MSAAMehod == EMSAAMethod::APPROACH_DISCONTINUITY);
	_updateShaderUniform("uAdaptiveShading", m_bAdaptiveShading);
	_updateShaderUniform("uShowComplexPixels", m_bMarkComplexPixels);
	_updateShaderUniform("uLightingModel", m_BRDF);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[0]);
	_updateShaderUniform("uNormalDepthTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[1]);
	_updateShaderUniform("uDiffuseEdgeTex", 1);

	// Render First (Simple Pixel) Pass
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilMask(0x00); // Disable writing to the stencil mask

	__renderScreenSizeQuad();

	_disableShader("MSAA_LIGHTING_SHADER");

	// Render Second (Complex Pixel) Pass
	glStencilFunc(GL_EQUAL, 0, 0xFF);

	if (m_bMarkComplexPixels)
	{
		_enableShader("MSAA_LIGHTING_MARK_COMPLEX_SHADER");
		__renderScreenSizeQuad();
		_disableShader("MSAA_LIGHTING_MARK_COMPLEX_SHADER");
	}
	else
	{
		_enableShader("MSAA_LIGHTING_PERSAMPLE_SHADER");
		_updateShaderUniform("uProjViewMatrix", glm::inverse(m_ProjectionMatrix));
		_updateShaderUniform("uViewWorldMatrix", glm::inverse(m_pInputTransformer->getModelViewMat()));
	//	_updateShaderUniform("uSecondPass", true);
		_updateShaderUniform("uLightingModel", m_BRDF);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[0]);
		_updateShaderUniform("uNormalDepthTex", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_pMSAAFBO->m_GBufferTex[1]);
		_updateShaderUniform("uDiffuseEdgeTex", 1);

		glEnable(GL_SAMPLE_SHADING);
		__renderScreenSizeQuad();
		glDisable(GL_SAMPLE_SHADING);

		_disableShader("MSAA_LIGHTING_PERSAMPLE_SHADER");
	}

	// Now resolve from our off-screen MSAA lighting accumulation buffer to our final, presentable framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pMSAAFBO->m_OffScreenFboId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	
	glBlitFramebuffer(0, 0, m_pMSAAFBO->m_ImageWidth, m_pMSAAFBO->m_ImageHeight, 0, 0, m_pMSAAFBO->m_ImageWidth, m_pMSAAFBO->m_ImageHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__updateRenderTargets()
{
	if (m_MSAAMehod == EMSAAMethod::APPROACH_NOMSAA)
	{
		if (m_pMSAAFBO->m_CurrentRendeTargetMode != CMSAAFBO::ERenderTargetMode::RTMODE_NONE)
		{
			__destoryRenderTargets();
			m_pMSAAFBO->initRenderTarget_NoMSAA();
		}
	}
	else if (m_bUsePerSamplePixelShader)
	{
		if (m_pMSAAFBO->m_CurrentRendeTargetMode != CMSAAFBO::ERenderTargetMode::RTMODE_PERSAMPLE)
		{
			__destoryRenderTargets();
			m_pMSAAFBO->initRenderTarget_PerSampleShading();
		}
	}
	else
	{
		if (m_pMSAAFBO->m_CurrentRendeTargetMode != CMSAAFBO::ERenderTargetMode::RTMODE_PERPIXEL)
		{
			__destoryRenderTargets();
			m_pMSAAFBO->initRenderTarget_PerPixelShading();
		}
	}
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__destoryRenderTargets()
{
	if (m_pMSAAFBO->m_CurrentRendeTargetMode != CMSAAFBO::ERenderTargetMode::RTMODE_NONE)
	{
		m_pMSAAFBO->destroyRenderTargets();
	}
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__renderScreenSizeQuad()
{
//	_updateShaderUniform("uModelViewMatrix", glm::mat4(1.0));
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//***********************************************************
//FUNCTION:
void CMSAAEffect::__initSceneData()
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
//FUNCTION:
void CMSAAEffect::_destoryEffectV()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::setMSAAMethod(const void *value, void * /*clientData*/)
{
	m_MSAAMehod = *static_cast<const int *>(value);
	if (m_MSAAMehod == 0)
	{
		m_bAdaptiveShading = false;
		m_bMarkComplexPixels = false;
		m_bSeparateComplexPass = false;
		m_bUsePerSamplePixelShader = false;
	}
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::getMSAAMethod(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = m_MSAAMehod;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::setPerSampleCB(const void *value, void * /*clientData*/)
{
	m_bUsePerSamplePixelShader = *static_cast<const int *>(value);
	if (m_bUsePerSamplePixelShader)
	{
		m_bAdaptiveShading = 0;
		m_bSeparateComplexPass = 1;
	}

	if (m_MSAAMehod == 0)
		m_MSAAMehod = 1;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::getPerSampleCB(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = m_bUsePerSamplePixelShader;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::setAdaptiveShadingCB(const void *value, void * /*clientData*/)
{
	m_bAdaptiveShading = *static_cast<const int *>(value);
	if (m_bAdaptiveShading)
		m_bUsePerSamplePixelShader = 0;

	if (m_MSAAMehod == 0)
		m_MSAAMehod = 1;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::getAdaptiveShadingCB(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = m_bAdaptiveShading;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::setSeparateComplexPassCB(const void *value, void * /*clientData*/)
{
	m_bSeparateComplexPass = *static_cast<const int *>(value);
	if (!m_bSeparateComplexPass)
		m_bUsePerSamplePixelShader = 0;

	if (m_MSAAMehod == 0)
		m_MSAAMehod = 1;
}

//***********************************************************
//FUNCTION:
void TW_CALL CMSAAEffect::getSeparateComplexPassCB(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = m_bSeparateComplexPass;
}