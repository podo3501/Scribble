#pragma once
#include <vector>
#include <memory>
#include <string>
#include <DirectXCollision.h>

class CGeometry;
struct Vertex;
struct Geometry;

class CModel
{
public:
	template<typename T>
	CModel(T&& resPath)
		: m_name("skullGeo")
		, m_submeshName("skull")
		, m_resPath(std::forward<T>(resPath))
	{}

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool Read();
	bool Convert(CGeometry* geometry);

	inline std::string GetName() { return m_name; };
	inline std::string GetSubmeshName() { return m_submeshName; };

private:
	std::string m_name{};
	std::string m_submeshName{};
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Models/" };

	std::vector<Vertex> m_vertices{};
	std::vector<std::int32_t> m_indices{};
	DirectX::BoundingBox m_boundingBox{};
	DirectX::BoundingSphere m_boundingSphere{};
};
