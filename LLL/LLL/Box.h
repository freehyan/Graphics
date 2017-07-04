#pragma once
#include <assimp/scene.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "../../Graphics/Graphics/BaseProduct.h"

enum EJudge
{
	OUT_BOX = 0,
	IN_BOX
};

class CBox :public GraphicsCommon::CBaseProduct
{
public:
	CBox() {}
	CBox(int vStatus) { m_JudgeStatus = vStatus; }
	virtual ~CBox() {}

	virtual bool genBox4MeshV(const std::string& vModelFileName, const aiMesh* vaiMesh, float vScale) { return false; }
	virtual bool isInBoxV(glm::vec3 vPoint) { return false; }

	virtual void genBoxVertexDataV(std::vector<glm::vec3>& voSet) {}

	void setModelName(const std::string& vModelName) { m_ModelName = vModelName; }
	void setJudgeStatus(int vStatus) { m_JudgeStatus = vStatus; }

	const std::string& getModelName() { return m_ModelName; }
	const int		   getJudgeStatus() { return m_JudgeStatus; }

private:
	virtual bool __isBoxEmptyV() { return true; }

	int m_JudgeStatus;
	std::string m_ModelName;
};