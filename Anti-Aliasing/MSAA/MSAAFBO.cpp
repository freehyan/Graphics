#include "MSAAFBO.h"

CMSAAFBO::CMSAAFBO() : m_ImageWidth(1280), m_ImageHeight(720), m_iMSAACount(4), m_CurrentRendeTargetMode(ERenderTargetMode::RTMODE_NONE)
{
}


CMSAAFBO::~CMSAAFBO()
{
	__releaseRenderTargets();
}

//***********************************************************
//FUNCTION:
void CMSAAFBO::initRenderTarget_NoMSAA()
{
	{
		glGenTextures(2, m_GBufferTex);

		glBindTexture(GL_TEXTURE_RECTANGLE, m_GBufferTex[0]);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F, m_ImageWidth, m_ImageHeight, 0, GL_RGBA, GL_FLOAT, 0);

		glBindTexture(GL_TEXTURE_RECTANGLE, m_GBufferTex[1]);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F, m_ImageWidth, m_ImageHeight, 0, GL_RGBA, GL_FLOAT, 0);
	}

	glGenTextures(1, &m_DepthStencilTex);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_DepthStencilTex);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH24_STENCIL8, m_ImageWidth, m_ImageHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glGenFramebuffers(1, &m_GBufferFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, m_GBufferTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, m_GBufferTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_DepthStencilTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


//***********************************************************
//FUNCTION:
void CMSAAFBO::initRenderTarget_PerPixelShading()
{
	__createRenderTargets();

	m_LightingAccumulationBufferFboId = 0;
	m_CurrentRendeTargetMode = ERenderTargetMode::RTMODE_PERPIXEL;
}

//***********************************************************
//FUNCTION:
void CMSAAFBO::initRenderTarget_PerSampleShading()
{
	__createRenderTargets();

	// Create the off-screen MSAA surface for rendering the per-sample lighting pass into
	glGenTextures(1, &m_OffScreenMSAATex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_OffScreenMSAATex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_iMSAACount, GL_RGBA16F, m_ImageWidth, m_ImageHeight, false);
	
	glGenFramebuffers(1, &m_OffScreenFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_OffScreenFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_OffScreenMSAATex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_DepthStencilTex, 0);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Lighting will be done to the off-screen MSAA frame buffer
	m_LightingAccumulationBufferFboId = m_OffScreenFboId;
	m_CurrentRendeTargetMode = ERenderTargetMode::RTMODE_PERSAMPLE;
}

//***********************************************************
//FUNCTION:
void CMSAAFBO::__releaseRenderTargets()
{
	glDeleteFramebuffers(1, &m_OffScreenFboId);
	m_OffScreenFboId = 0;
	glDeleteFramebuffers(1, &m_ResolveFboId);
	m_ResolveFboId = 0;
	glDeleteFramebuffers(1, &m_GBufferFboId);
	m_GBufferFboId = 0;
	glDeleteTextures(1, &m_OffScreenMSAATex);
	m_OffScreenMSAATex = 0;
	glDeleteTextures(1, &m_ResolveColorTex);
	m_ResolveColorTex = 0;

	glDeleteTextures(1, &m_DepthStencilTex);
	m_DepthStencilTex = 0;
	glDeleteTextures(3, m_GBufferTex);
	m_GBufferTex[0] = 0;
	m_GBufferTex[1] = 0;
	m_GBufferTex[2] = 0;
}

//***********************************************************
//FUNCTION:
void CMSAAFBO::__createRenderTargets()
{
	glEnable(GL_MULTISAMPLE);

	{
		glGenTextures(3, m_GBufferTex);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[0]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_iMSAACount, GL_RGBA16F, m_ImageWidth, m_ImageHeight, false);

		// Buffer 1 holds Color and a flag indicating Edge pixels
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[1]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_iMSAACount, GL_RGBA16F, m_ImageWidth, m_ImageHeight, false);

		// Buffer 2 holds a copy of the sample's Coverage Mask 
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[2]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_iMSAACount, GL_R8UI, m_ImageWidth, m_ImageHeight, false);
	}

	// Create the Depth/Stencil
	glGenTextures(1, &m_DepthStencilTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_DepthStencilTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_iMSAACount, GL_DEPTH24_STENCIL8, m_ImageWidth, m_ImageHeight, false);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glGenFramebuffers(1, &m_GBufferFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, m_GBufferTex[2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_DepthStencilTex, 0);

	glGenTextures(1, &m_ResolveColorTex);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_ResolveColorTex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F, m_ImageWidth, m_ImageHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glGenFramebuffers(1, &m_ResolveFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ResolveFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, m_ResolveColorTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//***********************************************************
//FUNCTION:
void CMSAAFBO::destroyRenderTargets()
{
	__releaseRenderTargets();

	m_LightingAccumulationBufferFboId = 0;
	m_CurrentRendeTargetMode = ERenderTargetMode::RTMODE_NONE;
}