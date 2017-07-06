#pragma once
#include "FluidCommon.h"
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <vector>

class CFluid
{
public:
	CFluid(void);
	~CFluid(void);
	
	void initFluidParticles(const std::vector<glm::vec3>& vParticleSet);
	void renderPointSprite2DepthAndColor();

private:
	void __createFluidParticlesVertexBuffer();

	GLuint m_FluidParticlesVBO;
	unsigned int m_ParticleCount;
	SParticle* m_ParticleSet;
};