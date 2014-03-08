#ifndef TBMESH_H
#define TBMESH_H

#include <map>
#include <vector>
#include "Wm5Vector3.h"
#include "Wm5Transform.h"

using namespace Wm5;

class TBMesh
{
	public:
		TBMesh();
		~TBMesh();

		TBMesh& transformBy(Transform);

		void addTriangle(const Vector3f, const Vector3f, const Vector3f);

		const std::vector<Vector3f>& getVertices() const;
		const std::vector<int>& getIndices() const;

	private:
		std::string hashVertex(const Vector3f);
		int pushVectex(const Vector3f);

	private:
		int mVerticeNum;
		std::vector<Vector3f> mVertices;
		std::vector<int> mIndices;
		std::map<std::string, int> mIndexedVertices;
};

#endif