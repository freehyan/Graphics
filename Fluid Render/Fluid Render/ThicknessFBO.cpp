#include "ThicknessFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CThicknessFBO> theCreater("THICKNESS_FBO");

CThicknessFBO::CThicknessFBO()
{
}

CThicknessFBO::~CThicknessFBO()
{
}

//***********************************************************
//FUNCTION:
void CThicknessFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_thickness")
		_generateTexture(m_ThicknessTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
