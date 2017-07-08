#pragma once
#include <Cooler/FBO.h>

class CESMFBO : public Cooler::CFBO
{
public:
	CESMFBO();
	virtual ~CESMFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_PrefilterTexZ;

	friend class CLLLEffect;
};