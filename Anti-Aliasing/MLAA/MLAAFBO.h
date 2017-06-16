#pragma once
#include <Cooler/FBO.h>

class CMLAAFBO : public Cooler::CFBO
{
public:
	CMLAAFBO();
	virtual ~CMLAAFBO();

	virtual void addTextureV(const std::string& vTexName, GLint vInternalFormat /* = GL_RGBA32F */, GLenum vFormat /* = GL_RGBA */) override;

private:
	unsigned int m_EdgeDetectionTex, m_BlendingWeightTex, m_AntiAliasingTex;

friend class CMLAAEffect;
};