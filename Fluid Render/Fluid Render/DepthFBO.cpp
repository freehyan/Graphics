#include "DepthFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CDepthFBO> theCreater("DEPTH_FBO");

CDepthFBO::CDepthFBO()
{
}

CDepthFBO::~CDepthFBO()
{
}

//***********************************************************
//FUNCTION:
void CDepthFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_depth")
		_generateTexture(m_DepthTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
