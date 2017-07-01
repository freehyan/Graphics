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

struct SVertex
{
	glm::vec3 m_Pos;
	glm::vec2 m_Tex;

	SVertex(const glm::vec3& vPos, const glm::vec2& vTex)
	{
		m_Pos = vPos;
		m_Tex = vTex;
	}
};

class COITEffect : public Cooler::CEffect
{
public:
	COITEffect();
	virtual ~COITEffect();

protected:
	virtual void _initEffectV() override;
	virtual void _renderEffectV() override;
	virtual void _destoryEffectV() override;

private:
	void __initBufferObjects();
	void __resetBufferData();
	void __createSreenQuad();
	void __renderScreenSizeQuad();

	void __initTwoTriangles();
	void __drawTwoTriangles();

	void __renderTransparentScenePass();
	void __compositeFragmentListPass();


	GLuint m_ScreenWidth, m_ScreenHeight;
	GLuint m_FirstTrianglesVBO, m_SecondTrianglesVBO;

	GLuint m_StartOffsetTexture;
	GLuint m_StartOffsetBufferInitializer;
	GLuint m_AtomicCounterBuffer;
	GLuint m_FragmentLinkBuffer;
	GLuint m_FragmentListTexture;

	GLuint m_VBO, m_VAO;
	glm::mat4 m_ProjectionMatrix, m_ModelViewMatrix, m_NormalMatrix;
	Cooler::CInputTransformer* m_pInputTransformer;
};