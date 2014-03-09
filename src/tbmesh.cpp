
#include "tbmesh.h"
#include "Wm5APoint.h"

TBMesh::TBMesh()
{
	mVerticeNum = 0;
}

TBMesh::~TBMesh()
{

}

TBMesh *TBMesh::clone() const
{
	TBMesh *mesh = new0 TBMesh();
	mesh->mVerticeNum = mVerticeNum;
	mesh->mVertices.insert(mesh->mVertices.end(), mVertices.begin(), mVertices.end());
	mesh->mIndices.insert(mesh->mIndices.end(), mIndices.begin(), mIndices.end());

	std::map<std::string, int>::const_iterator it = mIndexedVertices.begin();
	for (; it != mIndexedVertices.end(); it++) {
		mesh->mIndexedVertices[it->first] = it->second;
	}
	return mesh;
}

std::string TBMesh::hashVertex(const Vector3f vertex)
{
	int x = int (vertex.X() * 10000);
	int y = int (vertex.Y() * 10000);
	int z = int (vertex.Z() * 10000);

	std::string hash;
	char stemp[100] = "";
	snprintf(stemp, 100, "%d-", x);
	hash += std::string(stemp);
	snprintf(stemp, 100, "%d-", y);
	hash += std::string(stemp);
	snprintf(stemp, 100, "%d", z);
	hash += std::string(stemp);

	return hash;
}

const std::vector<Vector3f>& TBMesh::getVertices() const
{
	return mVertices;
}

const std::vector<int>& TBMesh::getIndices() const
{
	return mIndices;
}

int TBMesh::pushVectex(const Vector3f p)
{
	std::string hash = hashVertex(p);
	std::map<std::string, int>::iterator it = mIndexedVertices.find(hash);
	if (it == mIndexedVertices.end()) {
		int index = mVerticeNum;
		mIndexedVertices[hash] = index;
		mVertices.push_back(p);
		mVerticeNum++;
		return index;
	}
	return it->second;
}

void TBMesh::addTriangle(const Vector3f p1, const Vector3f p2, const Vector3f p3)
{
	mIndices.push_back(pushVectex(p1));
	mIndices.push_back(pushVectex(p2));
	mIndices.push_back(pushVectex(p3));
}

TBMesh& TBMesh::transformBy(Transform xform)
{
	for (int i=0; i<mVertices.size(); i++) {
		Vector3f vertex = xform * (APoint)(mVertices.at(i));
		mVertices.at(i) = vertex;
	}
	return const_cast<TBMesh&>(*this);
}

