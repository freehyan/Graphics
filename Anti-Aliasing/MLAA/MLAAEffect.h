#pragma once
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GLM/glm.hpp>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/Effect.h>

namespace Cooler
{
	CInputTransformer;
}
class CDeferredFBO;
class CMLAAFBO;

class CMLAAEffect : public Cooler::CEffect
{
public:
	typedef enum { SRC_TEX = 0, EDGE_TEX, WEIGHT_TEX, ANTI_TEX } ETexType;
	typedef enum { SPONZA_MESH = 0, UNIGINE_MESH} EMeshType;

	CMLAAEffect();
	virtual ~CMLAAEffect();

protected:
	virtual void _initEffectV() override;
	virtual void _renderEffectV() override;
	virtual void _destoryEffectV() override;

private:
	void __initTweakBar();
	void __geometryPass();
	void __lightingPass();
	void __edgeDetecionPass();
	void __calcBlendingWeightPass();
	void __neighbourBlendPass();
	void __showTexturePass();

	void __renderScreenSizeQuad();
	void __initSceneData();

	glm::mat4 m_ProjectionMatrix, m_ModelViewMatrix, m_NormalMatrix;
	Cooler::CInputTransformer* m_pInputTransformer;
	CDeferredFBO* m_pDeferredFBO;
	CMLAAFBO* m_pMLAAFBO;
	GLuint m_VBO, m_VAO;
	GLuint m_PreComputedAreaTex;
	ETexType m_ShowTextureId;
	EMeshType m_ShowMeshId;
};