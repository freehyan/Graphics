#include "OITEffect.h"
#include <GLM/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <AntTweakBar/AntTweakBar.h>
#include <Cooler/ProductFactory.h>
#include <Cooler/GraphicsInterface.h>
#include <Cooler/InputTransformer.h>

Cooler::CProductFactory<COITEffect> theCreater("OIT_EFFECT");

COITEffect::COITEffect() : m_pInputTransformer(nullptr)
{
	m_ScreenWidth = Cooler::getDisplayWindowsSize().first;
	m_ScreenHeight = Cooler::getDisplayWindowsSize().second;
}

COITEffect::~COITEffect()
{
}

//***********************************************************
//FUNCTION:
void COITEffect::_initEffectV()
{
	_ASSERTE(glGetError() == GL_NO_ERROR);
	__createSreenQuad();

	m_pInputTransformer = Cooler::fetchInputTransformer();
	m_pInputTransformer->setTranslationVec(glm::vec3(0.0f, -5.0f, -5.0f));
	m_pInputTransformer->setScale(0.1f);
	m_pInputTransformer->setMotionMode(Cooler::SCameraMotionType::FIRST_PERSON);
	m_ProjectionMatrix = glm::perspective(glm::radians(60.0f), m_ScreenWidth / (float)m_ScreenHeight, 0.1f, 1000.0f);

	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	__initBufferObjects();
	__initTwoTriangles();

	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void COITEffect::_renderEffectV()
{
	glEnable(GL_CULL_FACE);

	m_ModelViewMatrix = m_pInputTransformer->getModelViewMat();
	m_NormalMatrix = m_pInputTransformer->getRotationMat();
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(m_ModelViewMatrix));

	__renderTransparentScenePass();
	__compositeFragmentListPass();

	
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void COITEffect::__renderScreenSizeQuad()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//***********************************************************
//FUNCTION::
void COITEffect::__initTwoTriangles()
{
	SVertex FirstTriangle[3] = { SVertex(glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec2(1.0, 0.0f)),
		SVertex(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec2(1.0, 0.0f)),
		SVertex(glm::vec3(-0.5f, 0.5f, -1.0f), glm::vec2(1.0, 0.0f)) };

	SVertex SecondTriangle[3] = { SVertex(glm::vec3(0.5f, -0.5f, -1.0f), glm::vec2(0.0, 1.0f)),
		SVertex(glm::vec3(0.5f, 0.5f, -1.0f), glm::vec2(0.0, 1.0f)),
		SVertex(glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec2(0.0, 1.0f)) };

	glGenBuffers(1, &m_FirstTrianglesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_FirstTrianglesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FirstTriangle), FirstTriangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_SecondTrianglesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_SecondTrianglesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SecondTriangle), SecondTriangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//***********************************************************
//FUNCTION::
void COITEffect::__drawTwoTriangles()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_FirstTrianglesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex), (const GLvoid*)12);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_SecondTrianglesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex), (const GLvoid*)12);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

//***********************************************************
//FUNCTION:
void COITEffect::_destoryEffectV()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

//***********************************************************
//FUNCTION::
void COITEffect::__initBufferObjects()
{
	GLuint* pData = nullptr;
	unsigned int ScreenTotalPixels = m_ScreenWidth * m_ScreenHeight;

	//生成start offset texture
	glGenTextures(1, &m_StartOffsetTexture);
	glBindTexture(GL_TEXTURE_2D, m_StartOffsetTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_ScreenWidth, m_ScreenHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//每帧都需要重初始化StartOffsetBuffer,使用PBO实现
	glGenBuffers(1, &m_StartOffsetBufferInitializer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_StartOffsetBufferInitializer);//GL_PIXEL_UNPACK_BUFFER通常作为glTexImage2D等纹理命令数据来源
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

	glGenBuffers(1, &m_FragmentLinkBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, m_FragmentLinkBuffer);
	glBufferData(GL_TEXTURE_BUFFER, 5 * ScreenTotalPixels * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glGenTextures(1, &m_FragmentListTexture);	//创建TBO
	glBindTexture(GL_TEXTURE_BUFFER, m_FragmentListTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, m_FragmentLinkBuffer); //绑定到缓存和设定格式
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

//***********************************************************
//FUNCTION::
void COITEffect::__resetBufferData()
{
	GLuint* pData = nullptr;
	//重置atomic counter
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_AtomicCounterBuffer);
	pData = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
	pData[0] = 0;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	
	//清除start offset buffer image
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_StartOffsetBufferInitializer);
	glBindTexture(GL_TEXTURE_2D, m_StartOffsetTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_ScreenWidth, m_ScreenHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

//***********************************************************
//FUNCTION::
void COITEffect::__createSreenQuad()
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
void COITEffect::__renderTransparentScenePass()
{
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//必须重置数据
	__resetBufferData();

	_enableShader("OIT_CREATE_LINKEDLIST_SHADER");
	glBindImageTexture(0, m_StartOffsetTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, m_FragmentListTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);

	glm::mat4 MVPMatrix = m_ProjectionMatrix * m_ModelViewMatrix;

	_updateShaderUniform("uMVPMatrix", MVPMatrix);
	_updateShaderUniform("uNormalMatrix", m_NormalMatrix);
	Cooler::graphicsRenderModel("CHESS");
	//__drawTwoTriangles();
	_disableShader("OIT_CREATE_LINKEDLIST_SHADER");
}

//***********************************************************
//FUNCTION::
void COITEffect::__compositeFragmentListPass()
{
	_enableShader("OIT_TRAVERSE_LINKEDLIST_SHADER");
	__renderScreenSizeQuad();
	_disableShader("OIT_TRAVERSE_LINKEDLIST_SHADER");
}
