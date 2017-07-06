#pragma once
#include <Cooler/FBO.h>

class CCompositeFBO : public Cooler::CFBO
{
public:
	CCompositeFBO();
	virtual ~CCompositeFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_CompositeTex;

	friend class CFluidEffect;
};