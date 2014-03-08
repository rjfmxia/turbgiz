#ifndef _TBMESHBOOLEAN_
#define _TBMESHBOOLEAN_

#include "tbmesh.h"

class TBBoolean
{
public:
	static void add(const TBMesh &m1, const TBMesh &m2, TBMesh &result);
	static void sub(const TBMesh &m1, const TBMesh &m2, TBMesh &result);
	static void diff(const TBMesh &m1, const TBMesh &m2, TBMesh &result);

	// Use for testing only.
	static void testMeshConvert(const TBMesh &mesh, TBMesh &result);
};
#endif