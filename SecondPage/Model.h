#pragma once

struct MeshGeometry;

class CModel
{
public:
	CModel();
	bool Read(MeshGeometry* meshGeo);

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

private:
};
