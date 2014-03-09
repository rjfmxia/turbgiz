
#include "tbmeshboolean.h"
#include "tbmesh.h"

extern "C" {
    #include "gts.h"
}

namespace {

static std::string getEdgeKey(int ia, int ib) {
	std::string key;
	char stemp[200] = "";
	snprintf(stemp, 200, "%d-", ia);
	key += std::string(stemp);
	snprintf(stemp, 200, "%d", ib);
	key += std::string(stemp);
	return key;
}

static void build_list (gpointer data, GSList ** list)
{
  /* always use O(1) g_slist_prepend instead of O(n) g_slist_append */
  *list = g_slist_prepend (*list, data);
}

static void prepend_triangle_bbox (GtsTriangle * t, GSList ** bboxes)
{
  *bboxes = g_slist_prepend (*bboxes, gts_bbox_triangle (gts_bbox_class (), t));
}

static void tbMeshFromGtsSurface(GtsSurface * s, TBMesh &mesh)
{
	/* build list of triangles */
	GSList * triangles = NULL;
	gts_surface_foreach_face (s, (GtsFunc)build_list, &triangles);
	GSList * i = triangles;
	while (i)
	{
		GtsEdge * e1, * e2, * e3;
		GtsVertex * p1, * p2, * p3;
		GtsTriangle * t = (GtsTriangle *)i->data;
		gts_triangle_vertices_edges (t, NULL, &p1, &p2, &p3, &e1, &e2, &e3);

		Vector3f v1( GTS_POINT(p1)->x, GTS_POINT(p1)->y, GTS_POINT(p1)->z);
		Vector3f v2( GTS_POINT(p2)->x, GTS_POINT(p2)->y, GTS_POINT(p2)->z);
		Vector3f v3( GTS_POINT(p3)->x, GTS_POINT(p3)->y, GTS_POINT(p3)->z);
		mesh.addTriangle(v1, v2, v3);

		i = i->next;
	}

	/* free list of triangles */
	g_slist_free (triangles);
}

static GtsSurface * gtsSurfaceFromTBMesh(const TBMesh &mesh)
{
	const std::vector<Vector3f>& vertices = mesh.getVertices() ;
	const std::vector<int>& indices = mesh.getIndices();

	GtsSurface *s = gts_surface_new (gts_surface_class (),
					GTS_FACE_CLASS (gts_nface_class ()),
					GTS_EDGE_CLASS (gts_nedge_class ()),
					GTS_VERTEX_CLASS (gts_nvertex_class ()));

	// Create gts vertices.
	std::vector<GtsVertex *> gtsVertices;
	std::vector<Vector3f>::const_iterator vit = vertices.begin();
	for (; vit != vertices.end(); vit++) {
		Vector3f v = *vit;
		GtsVertex * gv = gts_vertex_new (s->vertex_class, v.X(), v.Y(), v.Z());
		gtsVertices.push_back(gv);
	}

	std::map<std::string, GtsEdge*> edgeMap;

	// Create gts edges.
	std::vector<int>::const_iterator it = indices.begin();
	for (; it != indices.end(); it+=3) {
		int i1 = *it;
		int i2 = *(it+1);
		int i3 = *(it+2);

		GtsEdge *edges[3];
		int edgeIndices[] = {i1, i2, i2, i3, i3, i1};
		for (int i=0; i<3; i++) {
			int e1 = edgeIndices[i*2];
			int e2 = edgeIndices[i*2+1];
			std::string key1 = getEdgeKey(e1, e2);
			std::string key2 = getEdgeKey(e2, e1);
			std::map<std::string, GtsEdge*>::iterator it = edgeMap.find(key1);
			if (it != edgeMap.end()) {
				edges[i] = it->second;
			} else {
				GtsEdge *edge = gts_edge_new (s->edge_class, gtsVertices[e1], gtsVertices[e2]);
				edgeMap[key1] = edge;
				edgeMap[key2] = edge;
				edges[i] = edge;
			}
		}

		// Add face.
		gts_surface_add_face (s, gts_face_new (s->face_class, edges[0], edges[1], edges[2]));
	}
	return s;
}
}

void TBBoolean::add(const TBMesh &m1, const TBMesh &m2, TBMesh &result)
{
	GtsSurface *s1 = gtsSurfaceFromTBMesh(m1);
	GtsSurface *s2 = gtsSurfaceFromTBMesh(m2);

	/* check surfaces */
	g_assert (gts_surface_is_orientable (s1));
	g_assert (gts_surface_is_orientable (s2));
	g_assert (!gts_surface_is_self_intersecting (s1));
	g_assert (!gts_surface_is_self_intersecting (s2));

	/* build bounding boxes for first surface */
	GSList *bboxes = NULL;
	gts_surface_foreach_face (s1, (GtsFunc) prepend_triangle_bbox, &bboxes);
	/* build bounding box tree for first surface */
	GNode *tree1 = gts_bb_tree_new (bboxes);
	/* free list of bboxes */
	g_slist_free (bboxes);
	gboolean is_open1 = gts_surface_volume (s1) < 0. ? TRUE : FALSE;

	/* build bounding boxes for second surface */
	bboxes = NULL;
	gts_surface_foreach_face (s2, (GtsFunc) prepend_triangle_bbox, &bboxes);
	/* build bounding box tree for second surface */
	GNode *tree2 = gts_bb_tree_new (bboxes);
	/* free list of bboxes */
	g_slist_free (bboxes);
	gboolean is_open2 = gts_surface_volume (s2) < 0. ? TRUE : FALSE;

	/* boolean surface */
	GtsSurfaceInter *si = gts_surface_inter_new (gts_surface_inter_class (), 
				s1, s2, tree1, tree2, is_open1, is_open2);

	GtsSurface * s3 = gts_surface_new (gts_surface_class (),
										gts_face_class (),
										gts_edge_class (),
										gts_vertex_class ());

	gts_surface_inter_boolean (si, s3, GTS_1_OUT_2);
	gts_surface_inter_boolean (si, s3, GTS_2_OUT_1);

	/* get result from s3 */
	tbMeshFromGtsSurface(s3, result);

	/* destroy surfaces and intersection */
	gts_object_destroy (GTS_OBJECT (s1));
	gts_object_destroy (GTS_OBJECT (s2));
	gts_object_destroy (GTS_OBJECT (s3));
	gts_object_destroy (GTS_OBJECT (si));

	/* destroy bounding box trees (including bounding boxes) */
	gts_bb_tree_destroy (tree1, TRUE);
	gts_bb_tree_destroy (tree2, TRUE);
}


void TBBoolean::sub(const TBMesh &m1, const TBMesh &m2, TBMesh &result)
{
	// TODO:
}

void TBBoolean::diff(const TBMesh &m1, const TBMesh &m2, TBMesh &result)
{
	// TODO:
}

void TBBoolean::testMeshConvert(const TBMesh &mesh, TBMesh &result)
{
	GtsSurface *s = gtsSurfaceFromTBMesh(mesh);
	g_assert (gts_surface_is_orientable (s));
	g_assert (!gts_surface_is_self_intersecting (s));
	tbMeshFromGtsSurface(s, result);
	gts_object_destroy (GTS_OBJECT (s));
}
