#include "Fluid.h"

CFluid::CFluid(void) : m_ParticleSet(nullptr), m_ParticleCount(0)
{
}

CFluid::~CFluid(void)
{
}

//***********************************************************
//FUNCTION::
void CFluid::initFluidParticles(const std::vector<glm::vec3>& vParticleSet)
{
	_ASSERTE(!vParticleSet.empty());

	m_ParticleCount = vParticleSet.size();
	m_ParticleSet = new SParticle[m_ParticleCount];
	unsigned int i = 0;
	for (auto particle : vParticleSet)
	{
		m_ParticleSet[i].mPos = particle;
		i++;
	}

	__createFluidParticlesVertexBuffer();
}

//***********************************************************
//FUNCTION::
void CFluid::renderPointSprite2DepthAndColor()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_POINT_SPRITE);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_FluidParticlesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SParticle), 0);
	glDrawArrays(GL_POINTS, 0, m_ParticleCount);

	glDisableVertexAttribArray(0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_DEPTH_TEST);
}

//***********************************************************
//FUNCTION::
void CFluid::__createFluidParticlesVertexBuffer()
{
	_ASSERTE(m_ParticleSet);
	glGenBuffers(1, &m_FluidParticlesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_FluidParticlesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SParticle)*m_ParticleCount, m_ParticleSet, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}