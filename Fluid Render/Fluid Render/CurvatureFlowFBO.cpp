#include "CurvatureFlowFBO.h"
#include <Cooler/ProductFactory.h>

Cooler::CProductFactory<CCurvatureFlowFBO> theCreater("CURVATURE_FLOW_FBO");

CCurvatureFlowFBO::CCurvatureFlowFBO()
{
}

CCurvatureFlowFBO::~CCurvatureFlowFBO()
{
}

//***********************************************************
//FUNCTION:
void CCurvatureFlowFBO::addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */)
{
	if (vTexName == "tex_smooth_depth")
		_generateTexture(m_SmoothDepthTex, vInternalFormat, vFormat);
	else
		std::cout << vTexName + " is not defined in the resource config file." << std::endl;
}
