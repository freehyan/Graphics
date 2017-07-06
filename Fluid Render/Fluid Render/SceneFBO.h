#pragma once
#include <Cooler/FBO.h>

class CSceneFBO : public Cooler::CFBO
{
public:
	CSceneFBO();
	virtual ~CSceneFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_SceneTex;

	friend class CFluidEffect;
};