#pragma once
#include <vector>
#include "Singleton.h"
#include "Box.h"

class CBoxManager : public GraphicsCommon::CSingleton<CBoxManager>
{
public:
	~CBoxManager(void);

	bool genBox(const std::string& vModelFileName, int vStatus, const std::string& vBoxSig, float vScale);
	bool isPointPositionValid(glm::vec3 vPoint);

	void renderBoxsetBorder();

	void initVBOAndVAO();

protected:
	CBoxManager(void);

private:
	bool __isModelBoxExist(const std::string& vModelFileName);

	std::vector<CBox*> m_Boxset;

	std::vector<glm::vec3> m_BoxVertices;

	unsigned int* m_Indices;

	GLuint m_VB;
	GLuint m_IB;

	friend class GraphicsCommon::CSingleton<CBoxManager>;
	friend class CLightManager;
};