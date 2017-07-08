#pragma once
#include <Cooler/FBO.h>

class CSrcSMFBO : public Cooler::CFBO
{
public:
	CSrcSMFBO();
	virtual ~CSrcSMFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_ESMTex;

	friend class CLLLEffect;
};