#include "ShadedNormalFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CShadedNormalFBO> theCreater("SHADED_NORMAL_FBO");

CShadedNormalFBO::CShadedNormalFBO()
{
}

CShadedNormalFBO::~CShadedNormalFBO()
{
}

//***********************************************************
//FUNCTION:
void CShadedNormalFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_shaded_normal")
		_generateTexture(m_ShadedNormalTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
