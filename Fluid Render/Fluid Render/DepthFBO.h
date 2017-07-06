#pragma once
#include <Cooler/FBO.h>

class CDepthFBO : public Cooler::CFBO
{
public:
	CDepthFBO();
	virtual ~CDepthFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_DepthTex;

	friend class CFluidEffect;
};