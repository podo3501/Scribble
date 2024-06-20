#pragma once
#include <vector>
#include <memory>
#include <string>
#include <DirectXCollision.h>

class CGeometry;
struct Geometry;

class CModel
{
public:
	CModel();

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool Read(CGeometry* geometry);

	inline std::string GetName() { return m_name; };
	inline std::string GetSubmeshName() { return m_submeshName; };

private:
	std::string m_name{};
	std::string m_submeshName{};
};
