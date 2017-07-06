#pragma once
#include <Cooler/FBO.h>

class CBilateralNormalFBO : public Cooler::CFBO
{
public:
	CBilateralNormalFBO();
	virtual ~CBilateralNormalFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_BilateralNormalTex;

	friend class CFluidEffect;
};