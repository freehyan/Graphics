#pragma once
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GLM/glm.hpp>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/Effect.h>
#include "GPULightEnv.h"
#include "LightStructure.h"

#define FMAX(a, b) a > b ? a : b

namespace Cooler
{
	CInputTransformer;
}
class CDeferredFBO;
class CMLAAFBO;

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
	void __initDirectionLight();
	void __initTransparentMeshPosition();
	void __initLLLUAVBuffers();
	void __initLightSourcesData();
	void __createSreenQuad();
	void __createLightSSBO();

	void __DSGemetroyPass();
	void __createLLLCullingBackFacePass();
	void __createLLLCullingFrontFacePass();
	void __renderOpaqueScenePass();
	void __DSDirectionalLightingPass();
	void __renderTransparentScenePass();

	
	void __resetLightStartOffsetBuffer();
	float __calcPointLightBSphere(const SPointLight& vLight);
	void __updateLightSourceSetPosition();
	void __setDirectionalLight();

	SDirectionalLight m_DirectinalLight;
	std::vector<glm::vec3> m_BottlePosition;
	std::vector<SPointLight*> m_PointLightSourceSet;
	std::vector<float> m_InitDegree;

	//LLL UAV Buffers
	GLuint m_LightStartOffsetTex;
	GLuint m_LightStartOffsetBufferPBO;
	GLuint m_AtomicCounterBuffer;
	GLuint m_LightFragmentLinkedBuffer;
	GLuint m_LightFragmentLinkedTexture;

	void __initTweakBar();
	void __renderScreenSizeQuad();

	GLuint m_LightSSBO;
	GLuint m_SSBOBlockIndex;
	GLuint m_ScreenWidth, m_ScreenHeight;
	GLboolean m_bMoveLightSource;
	GLint m_Count;

	glm::mat4 m_ProjectionMatrix, m_ModelViewMatrix, m_NormalMatrix;
	Cooler::CInputTransformer* m_pInputTransformer;
	CDeferredFBO* m_pDeferredFBO;
	GLuint m_VBO, m_VAO;
};