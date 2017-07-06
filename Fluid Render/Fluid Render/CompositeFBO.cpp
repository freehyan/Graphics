#include "CompositeFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CCompositeFBO> theCreater("COMPOSITE_FBO");

CCompositeFBO::CCompositeFBO()
{
}

CCompositeFBO::~CCompositeFBO()
{
}

//***********************************************************
//FUNCTION:
void CCompositeFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_composite")
		_generateTexture(m_CompositeTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
