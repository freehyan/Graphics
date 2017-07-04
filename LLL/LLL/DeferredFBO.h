#pragma once
#include <Cooler/FBO.h>

class CDeferredFBO : public Cooler::CFBO
{
public:
	CDeferredFBO();
	virtual ~CDeferredFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_PosTex, m_NormalTex, m_ColorTex, m_FinalTex;

	friend class CLLLEffect;
};