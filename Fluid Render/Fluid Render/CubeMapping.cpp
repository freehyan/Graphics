#include "CubeMapping.h"
#include <SOIL/SOIL.h>

CCubeMapping::CCubeMapping()
{
}

CCubeMapping::~CCubeMapping()
{
}

//***********************************************************
//FUNCTION:
void CCubeMapping::initCubeMappingTextureSet(const std::vector<std::string>& vTextureSetName)
{
	_ASSERTE(!vTextureSetName.empty());
	glGenTextures(1, &m_CubemapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);
	
	int Width = 0, Height = 0;
	unsigned char* pImageData = nullptr;

	for (unsigned int i=0; i<vTextureSetName.size(); i++)
	{
		pImageData = SOIL_load_image(vTextureSetName[i].c_str(), &Width, &Height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
		SOIL_free_image_data(pImageData);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	__onInitSkyBox();
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION:
void CCubeMapping::renderSkybox()
{
	glBindVertexArray(m_SkyboxVAO);
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	_ASSERTE(glGetError() == GL_NO_ERROR);
}

//***********************************************************
//FUNCTION::
void CCubeMapping::bindTexture(GLenum vTexUnit)
{
	glActiveTexture(vTexUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTex);
}

//***********************************************************
//FUNCTION:
void CCubeMapping::__onInitSkyBox()
{
	GLfloat SkyboxVertices[] = { 
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &m_SkyboxVAO);
	glBindVertexArray(m_SkyboxVAO);
	glGenBuffers(1, &m_SkyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
}