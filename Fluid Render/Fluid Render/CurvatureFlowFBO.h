#pragma once
#include <Cooler/FBO.h>

class CCurvatureFlowFBO : public Cooler::CFBO
{
public:
	CCurvatureFlowFBO();
	virtual ~CCurvatureFlowFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_SmoothDepthTex;

	friend class CFluidEffect;
};