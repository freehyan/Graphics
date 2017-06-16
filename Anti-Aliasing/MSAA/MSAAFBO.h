#pragma once
#include <gl/glew.h>
#include <gl/freeglut.h>

class CMSAAFBO
{
public:
	enum ERenderTargetMode
	{
		RTMODE_NONE,
		RTMODE_PERPIXEL,
		RTMODE_PERSAMPLE
	};

public:
	CMSAAFBO();
	~CMSAAFBO();

	void initRenderTarget_NoMSAA();
	void initRenderTarget_PerPixelShading();
	void initRenderTarget_PerSampleShading();

	void destroyRenderTargets();

private:
	void __releaseRenderTargets();
	void __createRenderTargets();

	GLuint m_GBufferTex[3]; //Normal_Depth, Color, the sample's coverage mask
	GLuint m_GBufferFboId, m_DepthStencilTex;
	GLuint m_ResolveFboId, m_ResolveColorTex;
	GLuint m_OffScreenFboId, m_OffScreenMSAATex;
	
	GLuint m_LightingAccumulationBufferFboId;
	ERenderTargetMode m_CurrentRendeTargetMode;

	unsigned int m_iMSAACount;
	const int m_ImageWidth, m_ImageHeight;

friend class CMSAAEffect;
};