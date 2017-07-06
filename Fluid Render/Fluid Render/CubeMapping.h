#pragma once
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>

class CCubeMapping
{
public:
	CCubeMapping();
	~CCubeMapping();

	void initCubeMappingTextureSet(const std::vector<std::string>& vTextureSetName);
	void renderSkybox();
	void bindTexture(GLenum vTexUnit);

private:
	void __onInitSkyBox();

	std::vector<std::string> m_CubeMapImageFileName;
	GLuint m_CubemapTex;

	GLuint m_SkyboxVAO, m_SkyboxVBO;
};