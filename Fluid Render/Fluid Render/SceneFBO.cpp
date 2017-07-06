#include "SceneFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CSceneFBO> theCreater("SCENE_FBO");

CSceneFBO::CSceneFBO()
{
}

CSceneFBO::~CSceneFBO()
{
}

//***********************************************************
//FUNCTION:
void CSceneFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_scene")
		_generateTexture(m_SceneTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
