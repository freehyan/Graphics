#include "MLAAFBO.h"
#include <Cooler/ProductFactory.h>

using namespace Cooler;
Cooler::CProductFactory<CMLAAFBO> theCreater("MLAA_FBO");

CMLAAFBO::CMLAAFBO()
{
}

CMLAAFBO::~CMLAAFBO()
{
}

//***********************************************************
//FUNCTION:
void CMLAAFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_edge_detection")
		_generateTexture(m_EdgeDetectionTex, vInternalFormat, vFormat);
	else if (vTexName == "tex_blending_weight")
		_generateTexture(m_BlendingWeightTex, vInternalFormat, vFormat);
	else if (vTexName == "tex_antialiasing")
		_generateTexture(m_AntiAliasingTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
