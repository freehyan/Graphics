#include "BilateralNormalFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CBilateralNormalFBO> theCreater("BILATERAL_NORMAL_FBO");

CBilateralNormalFBO::CBilateralNormalFBO()
{
}

CBilateralNormalFBO::~CBilateralNormalFBO()
{
}

//***********************************************************
//FUNCTION:
void CBilateralNormalFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_bilate_normal")
		_generateTexture(m_BilateralNormalTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
