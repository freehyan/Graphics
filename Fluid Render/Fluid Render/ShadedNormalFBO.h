#pragma once
#include <Cooler/FBO.h>

class CShadedNormalFBO : public Cooler::CFBO
{
public:
	CShadedNormalFBO();
	virtual ~CShadedNormalFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_ShadedNormalTex;

	friend class CFluidEffect;
};