#include "SrcSMFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CSrcSMFBO> theCreater("SRC_SM_FBO");

CSrcSMFBO::CSrcSMFBO()
{
}

CSrcSMFBO::~CSrcSMFBO()
{
}

//***********************************************************
//FUNCTION:
void CSrcSMFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_esm")
		_generateTexture(m_ESMTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
