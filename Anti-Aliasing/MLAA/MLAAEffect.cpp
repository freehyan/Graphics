#include "MLAAEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>
#include "DeferredFBO.h"
#include "MLAAFBO.h"

using namespace Cooler;
Cooler::CProductFactory<CMLAAEffect> theCreater("MLAA_EFFECT");
CMLAAEffect::CMLAAEffect() : m_pInputTransformer(nullptr), m_pDeferredFBO(nullptr), m_pMLAAFBO(nullptr), m_ShowTextureId(ANTI_TEX), m_ShowMeshId(SPONZA_MESH)
{
}


CMLAAEffect::~CMLAAEffect()
{
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::_initEffectV()
{
	__initTweakBar();
	__initSceneData();
	
	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(100.0f, -80.0f, 0.0f));
	m_pInputTransformer->setRotationVec(glm::vec3(0.0, 90.0, 0.0));
	m_pInputTransformer->setScale(0.1f);
	m_pInputTransformer->setMotionMode(Cooler::SCameraMotionType::FIRST_PERSON);

	//glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	m_pDeferredFBO = dynamic_cast<CDeferredFBO*>(Cooler::fetchFBO("DEFERRED_FBO"));
	_ASSERTE(m_pDeferredFBO);
	m_pMLAAFBO = dynamic_cast<CMLAAFBO*>(Cooler::fetchFBO("MLAA_FBO"));
	_ASSERTE(m_pMLAAFBO);
	m_ProjectionMatrix = glm::perspective(3.14f * 0.25f, 1280 / (float)720, 0.1f, 1000.0f);

	glEnable(GL_DEPTH_TEST);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::_renderEffectV()
{
	__geometryPass();
	__lightingPass();
	__edgeDetecionPass();
	__calcBlendingWeightPass();
	__neighbourBlendPass();
	__showTexturePass();
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::_destoryEffectV()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__initTweakBar()
{
	TwBar* pBar = Cooler::fetchTweakBar();
	_ASSERTE(pBar);

	TwEnumVal Textures[] = { { SRC_TEX , "Src Texture" },{ ANTI_TEX, "Anti-aliasing Tex" }, { EDGE_TEX, "Edge Texture" }, { WEIGHT_TEX, "Weight Texture" } };
	TwType TexTwType = TwDefineEnum("TexType", Textures, 4);

	TwAddVarRW(pBar, "Texture", TexTwType, &m_ShowTextureId, NULL);

	TwEnumVal Meshs[] = { { SPONZA_MESH , "Sponza Mesh" },{ UNIGINE_MESH, "Unigine Mesh" }};
	TwType MeshTwType = TwDefineEnum("MeshType", Meshs, 2);

	TwAddVarRW(pBar, "Mesh", MeshTwType, &m_ShowMeshId, NULL);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__geometryPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	m_pDeferredFBO->openFBO();
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glm::mat4 ModelMatrix;// = glm::scale(glm::mat4(1.0), glm::vec3(0.1f));;
	glm::mat4 ViewMatrix = m_pInputTransformer->getModelViewMat();

	glm::mat4 MVPMatrix = m_ProjectionMatrix * ViewMatrix * ModelMatrix;
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));
	_enableShader("MLAA_GEOMETRY_SHADER");

	_updateShaderUniform("uMVPMatrix", MVPMatrix);
	_updateShaderUniform("uModelMatrix", ModelMatrix);
	_updateShaderUniform("uNormalMatrix", NormalMatrix);

	Cooler::graphicsRenderModel("SPONZA");
	_disableShader("MLAA_GEOMETRY_SHADER");

	glDisable(GL_DEPTH_TEST);

	m_pDeferredFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__lightingPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	m_pDeferredFBO->openFBO();
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
//	glClear(GL_COLOR_BUFFER_BIT);
// 	float ClearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
// 	glClearBufferfv(GL_COLOR, 3, ClearColorZero);

	_enableShader("MLAA_LIGHTING_SHADER");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_NormalTex);
	_updateShaderUniform("uNormalTex", 0);
	__renderScreenSizeQuad();

	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("MLAA_LIGHTING_SHADER");
	m_pDeferredFBO->closeFBO();

	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__edgeDetecionPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	m_pMLAAFBO->openFBO();
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	_enableShader("EDGE_DETECTION_SHADER");

	if (m_ShowMeshId == SPONZA_MESH)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_LightingTex);
	}
	else
		_enableTexture("INPUT_TEXTURE", GL_TEXTURE0);
	
 	_updateShaderUniform("uColorTex", 0);
	__renderScreenSizeQuad();

	_disableTexture("INPUT_TEXTURE", GL_TEXTURE0);
	_disableShader("EDGE_DETECTION_SHADER");

	m_pMLAAFBO->closeFBO();
	glDisable(GL_DEPTH_TEST);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__calcBlendingWeightPass()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
  	m_pMLAAFBO->openFBO();
  	glDrawBuffer(GL_COLOR_ATTACHMENT1);

	_enableShader("BLENDING_WEIGHT_SHADER");

	_enableTexture("AREA_TEXTURE", GL_TEXTURE0);
	_updateShaderUniform("uPreComputedAreaTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pMLAAFBO->m_EdgeDetectionTex);
	_updateShaderUniform("uEdgeDetectionTex", 1);

	__renderScreenSizeQuad();
	_disableTexture("AREA_TEXTURE", GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("BLENDING_WEIGHT_SHADER");

	m_pMLAAFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__neighbourBlendPass()
{
	m_pMLAAFBO->openFBO();
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	_enableShader("NEIGHBORHOOD_BLENDING_SHADER");

	if (m_ShowMeshId == SPONZA_MESH)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_LightingTex);
	}
	else
		_enableTexture("INPUT_TEXTURE", GL_TEXTURE0);


	_updateShaderUniform("uColorTex", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pMLAAFBO->m_BlendingWeightTex);
	_updateShaderUniform("uBlendingWeightTex", 1);

	__renderScreenSizeQuad();
	_disableTexture("INPUT_TEXTURE", GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("NEIGHBORHOOD_BLENDING_SHADER");

	m_pMLAAFBO->closeFBO();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__showTexturePass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_enableShader("SHOW_TEXTURE_SHADER");

	if (m_ShowTextureId == SRC_TEX)
	{
		if (m_ShowMeshId == SPONZA_MESH)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_pDeferredFBO->m_LightingTex);
		}
		else
			_enableTexture("INPUT_TEXTURE", GL_TEXTURE0);
	}
	else if (m_ShowTextureId == EDGE_TEX)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pMLAAFBO->m_EdgeDetectionTex);
	}
	else if (m_ShowTextureId == WEIGHT_TEX)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pMLAAFBO->m_BlendingWeightTex);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pMLAAFBO->m_AntiAliasingTex);
	}

	_updateShaderUniform("uColorTex", 0);
	_updateShaderUniform("uShowMeshId", (int)m_ShowMeshId);
	__renderScreenSizeQuad();
	glBindTexture(GL_TEXTURE_2D, 0);
	_disableShader("SHOW_TEXTURE_SHADER");
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__renderScreenSizeQuad()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

//***********************************************************
//FUNCTION:
void CMLAAEffect::__initSceneData()
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

