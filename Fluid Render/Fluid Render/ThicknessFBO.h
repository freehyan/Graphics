#pragma once
#include <Cooler/FBO.h>

class CThicknessFBO : public Cooler::CFBO
{
public:
	CThicknessFBO();
	virtual ~CThicknessFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_ThicknessTex;

	friend class CFluidEffect;
};