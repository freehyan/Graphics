#include "BoxManager.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../../Graphics/Graphics/GraphicsInterface.h"

#define CUBE_POINT_NUM 8
#define CUBE_LINE_NUM 12
#define  LINE_POINT_NUM 2

CBoxManager::CBoxManager(void)
{
	m_Indices = nullptr;
}


CBoxManager::~CBoxManager(void)
{
	for (auto Box : m_Boxset)
	{
		delete Box;
	}
	m_Boxset.clear();

	delete m_Indices;
}


//***********************************************************
//FUNCTION:
bool CBoxManager::__isModelBoxExist(const std::string& vModelFileName)
{
	bool Ret = false;

	for (auto Box : m_Boxset)
	{
		if (vModelFileName == Box->getModelName())
		{
			Ret = true;
		}
	}

	return Ret;
}

//***********************************************************
//FUNCTION:
bool CBoxManager::genBox(const std::string& vModelFileName, int vStatus, const std::string& vBoxSig, float vScale)
{
	if (__isModelBoxExist(vModelFileName))
	{
		//error hint
		return false;
	}

	Assimp::Importer Importer;

	const aiScene* pScene = Importer.ReadFile(vModelFileName.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	_ASSERT(pScene);

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* paiMesh = pScene->mMeshes[i];
		CBox* pBox = dynamic_cast<CBox*>(GraphicsCommon::createProduct(vBoxSig));
		_ASSERT(pBox);

		if (!pBox->genBox4MeshV(vModelFileName, paiMesh, vScale))
		{
			return false;
		}
		pBox->setJudgeStatus(vStatus);
		m_Boxset.push_back(pBox);
	}
	return true;
}

//***********************************************************
//FUNCTION:
bool CBoxManager::isPointPositionValid(glm::vec3 vPoint)
{
	bool InRet = false;
	bool OutRet = true;

	for (auto Box : m_Boxset)
	{
		switch (Box->getJudgeStatus())
		{
		case IN_BOX:
			InRet = InRet || (Box->isInBoxV(vPoint) == Box->getJudgeStatus());
			break;
		case OUT_BOX:
			OutRet = OutRet && (Box->isInBoxV(vPoint) == Box->getJudgeStatus());
			break;
		default:
			break;
		}
	}
	return InRet && OutRet;
}

//***********************************************************
//FUNCTION:
void CBoxManager::renderBoxsetBorder()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glPointSize(5.0);
	glDrawElements(GL_LINES, CUBE_LINE_NUM * LINE_POINT_NUM * m_Boxset.size(), GL_UNSIGNED_INT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}

//***********************************************************
//FUNCTION:
void CBoxManager::initVBOAndVAO()
{
	unsigned int i = 0;

	m_Indices = new unsigned int[CUBE_LINE_NUM * LINE_POINT_NUM * m_Boxset.size()];

	int Line = CUBE_LINE_NUM * LINE_POINT_NUM;

	for (auto Box : m_Boxset)
	{
		Box->genBoxVertexDataV(m_BoxVertices);
		m_Indices[i * Line] = 0 + i * 8;
		m_Indices[i * Line + 1] = 1 + i * 8;
		m_Indices[i * Line + 2] = 0 + i * 8;
		m_Indices[i * Line + 3] = 3 + i * 8;
		m_Indices[i * Line + 4] = 0 + i * 8;
		m_Indices[i * Line + 5] = 4 + i * 8;
		m_Indices[i * Line + 6] = 1 + i * 8;
		m_Indices[i * Line + 7] = 2 + i * 8;
		m_Indices[i * Line + 8] = 1 + i * 8;
		m_Indices[i * Line + 9] = 5 + i * 8;
		m_Indices[i * Line + 10] = 2 + i * 8;
		m_Indices[i * Line + 11] = 3 + i * 8;
		m_Indices[i * Line + 12] = 2 + i * 8;
		m_Indices[i * Line + 13] = 6 + i * 8;
		m_Indices[i * Line + 14] = 3 + i * 8;
		m_Indices[i * Line + 15] = 7 + i * 8;
		m_Indices[i * Line + 16] = 4 + i * 8;
		m_Indices[i * Line + 17] = 7 + i * 8;
		m_Indices[i * Line + 18] = 4 + i * 8;
		m_Indices[i * Line + 19] = 5 + i * 8;
		m_Indices[i * Line + 20] = 5 + i * 8;
		m_Indices[i * Line + 21] = 6 + i * 8;
		m_Indices[i * Line + 22] = 6 + i * 8;
		m_Indices[i * Line + 23] = 7 + i * 8;
		i++;
	}

	glGenBuffers(1, &m_VB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * CUBE_POINT_NUM * m_Boxset.size(), &m_BoxVertices[0], GL_STATIC_DRAW);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * CUBE_LINE_NUM * LINE_POINT_NUM * m_Boxset.size(), &m_Indices[0], GL_STATIC_DRAW);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
