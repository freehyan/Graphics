#pragma once
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GLM/glm.hpp>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/Effect.h>
#include "Fluid.h"
#include "CubeMapping.h"

namespace Cooler
{
	CInputTransformer;
}
class CThicknessFBO;
class CDepthFBO;
class CCurvatureFlowFBO;
class CBilateralNormalFBO;
class CShadedNormalFBO;
class CSceneFBO;
class CCompositeFBO;

class CFluidEffect : public Cooler::CEffect
{
public:
	CFluidEffect();
	virtual ~CFluidEffect();

protected:
	virtual void _initEffectV() override;
	virtual void _renderEffectV() override;
	virtual void _destoryEffectV() override;

private:
	void __initDirectionLight();
	void __initFBOBuffers();
	void __createSreenQuad();

	void __calcThicknessPass();
	void __calcDepthPass();
	void __curtureFlowFilterPass();
	void __bilateralFilterPass();
	void __shadedNormalPass();
	void __scenePass();
	void __compositePass();
	void __showTexturePass(GLuint vTextureID);

	GLuint __generateTexture(unsigned int vTextureWidth, unsigned int vTextureHeight);


	CCubeMapping m_CubeMap;
	CFluid m_Fluid;
	SDirectionalLight m_DirectinalLight;
	std::vector<float> m_InitDegree;

	void __initTweakBar();
	void __renderScreenSizeQuad();

	GLuint m_VBO, m_VAO;
	GLuint m_ScreenWidth, m_ScreenHeight;
	GLfloat m_PointRadius, m_ThicknessScale;

	SDirectionalLight m_DirectionalLight;
	glm::mat4 m_ProjectionMatrix, m_ViewMatrix, m_WorldMatrix;
	Cooler::CInputTransformer* m_pInputTransformer;

	CThicknessFBO* m_pThickFBO;
	CDepthFBO* m_pDepthFBO;
	CCurvatureFlowFBO* m_pCurtureFlowFBO; //curvature
	CBilateralNormalFBO* m_pBilateralNormalFBO;
	CShadedNormalFBO* m_ShadedNormalFBO;
	CSceneFBO* m_pSceneFBO;
	CCompositeFBO* m_pCompositeFBO;
};