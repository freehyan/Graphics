#include "ESMFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CESMFBO> theCreater("ESM_FBO");

CESMFBO::CESMFBO()
{
}

CESMFBO::~CESMFBO()
{
}

//***********************************************************
//FUNCTION:
void CESMFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_prefilter_z")
		_generateTexture(m_PrefilterTexZ, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
