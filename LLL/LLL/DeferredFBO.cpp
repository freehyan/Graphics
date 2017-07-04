#include "DeferredFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CDeferredFBO> theCreater("DEFERRED_FBO");

CDeferredFBO::CDeferredFBO()
{
}

CDeferredFBO::~CDeferredFBO()
{
}

//***********************************************************
//FUNCTION:
void CDeferredFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_pos")
		_generateTexture(m_PosTex, vInternalFormat, vFormat);
	else if (vTexName == "tex_color")
		_generateTexture(m_ColorTex, vInternalFormat, vFormat);
	else if (vTexName == "tex_normal")
		_generateTexture(m_NormalTex, vInternalFormat, vFormat);
	else if (vTexName == "tex_final")
		_generateTexture(m_FinalTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
