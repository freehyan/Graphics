#pragma once
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GLM/glm.hpp>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/Effect.h>
#include "LightStructure.h"

namespace Cooler
{
	CInputTransformer;
}
class CSrcSMFBO;
class CESMFBO;

class CLLLEffect : public Cooler::CEffect
{
public:
	CLLLEffect();
	virtual ~CLLLEffect();

protected:
	virtual void _initEffectV() override;
	virtual void _renderEffectV() override;
	virtual void _destoryEffectV() override;

private:
	void __SrcShadowMapPass();
	void __ExponentialShadowMapPass();
	void __LightShadowPass();

	void __initSpotLight();
	void __setSpotLight(unsigned int vLightNum, const SSpotLight* vSpotLight);

	void __initTweakBar();
	void __createSreenQuad();
	void __renderScreenSizeQuad();

	SSpotLight* m_pSpotLight;

	GLuint m_ScreenWidth, m_ScreenHeight;

	glm::mat4 m_ProjectionMatrix, m_ViewMatrix, m_LightSpaceMVPMatrix;

	Cooler::CInputTransformer* m_pInputTransformer;
	CSrcSMFBO* m_pSrcSMFBO;
	CESMFBO* m_pESMFBO;
	GLuint m_VBO, m_VAO;
};