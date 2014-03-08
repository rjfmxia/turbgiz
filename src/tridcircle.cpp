

#include "tridcircle.h"
#include "Wm5Transform.h"

TridCircle::TridCircle(const Circle3f& circle1, const Circle3f& circle2, const Circle3f& circle3)
{
    m_Circles[0] = circle1;
    m_Circles[1] = circle2;
    m_Circles[2] = circle3;
}

TridCircle::~TridCircle()
{
}

void TridCircle::SetCircle1(const Circle3f& circle)
{
    m_Circles[0] = circle;
}

void TridCircle::SetCircle2(const Circle3f& circle)
{
    m_Circles[1] = circle;
}

void TridCircle::SetCircle3(const Circle3f& circle)
{
    m_Circles[2] = circle;
}

Circle3f TridCircle::GetCircle(int index)
{
    return m_Circles[index];
}

bool TridCircle::GetTangentPosition(const Circle2f& circle,
                                    const Line2f& line2,
                                    Vector3f& point)
{
    DistPoint2Line2f dist(circle.Center, line2);
    dist.GetSquared();

    Vector2f p = dist.GetClosestPoint0();
    point.X() = p.X();
    point.Y() = p.Y();
    point.Z() = 0;
    return true;
}

Circle2f TridCircle::ToCircle2f(const Circle3f& circle3)
{
    Vector2f center(circle3.Center.X(), circle3.Center.Y());
    return Circle2f(center, circle3.Radius);
}

Circle3f TridCircle::ToCircle3f(const Circle2f& circle2)
{
    Vector3f center(circle2.Center.X(), circle2.Center.Y(), 0);
    return Circle3f(center, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), circle2.Radius);
}

void TridCircle::AddUniquePoint(Vector3f position, std::vector<Vector3f>& vertices)
{
    float eps = 0.0001f;
    bool pointExist = false;
    std::vector<Vector3f>::iterator it = vertices.begin();
    for (; it != vertices.end(); it++) {
        Vector3f pos = *it;
        if (Mathf::FAbs(pos.X() - position.X()) < eps &&
            Mathf::FAbs(pos.Y() - position.Y()) < eps &&
            Mathf::FAbs(pos.Z() - position.Z()) < eps) {
            pointExist = true;
            break;
        }
    }

    if (!pointExist) {
        vertices.push_back(position);
    }
}

void TridCircle::GetTangentPointsToCircles(const Circle2f& cir0,
                                           const Circle2f& cir1,
                                           std::vector<Vector3f>& results)
{
    Line2f lines[4];
    if (GetTangentsToCircles(cir0, cir1, lines))
    {
        Vector3f point0, point1;
        Segment2f centerSeg(cir0.Center, cir1.Center);
        float eps = 0.0001f;
        for (int i=0; i<4; i++) {
            // Filter the inner lines.
            DistLine2Segment2f dist(lines[i], centerSeg);
            float distance = dist.GetSquared();
            if (distance > eps) {
                GetTangentPosition(cir0, lines[i], point0);
                AddUniquePoint(point0, results);
                GetTangentPosition(cir1, lines[i], point1);
                AddUniquePoint(point1, results);
            }
        }
    }
}

BSplineCurve3f *TridCircle::CreateCircle()
{
    Circle2f cir0 = ToCircle2f(m_Circles[0]);
    Circle2f cir1 = ToCircle2f(m_Circles[1]);
    Circle2f cir2 = ToCircle2f(m_Circles[2]);

    // Adding samples.
    std::vector<Vector3f> allSamples;
    GetTangentPointsToCircles(cir0, cir1, allSamples);
    GetTangentPointsToCircles(cir0, cir2, allSamples);
    GetTangentPointsToCircles(cir1, cir2, allSamples);

    // Adding samples.
    int sampleNum = 20;
    TessellateCircle(cir0, sampleNum, allSamples);
    TessellateCircle(cir1, sampleNum, allSamples);
    TessellateCircle(cir2, sampleNum, allSamples);

    // Vector to array.
    int len = allSamples.size();
    Vector3f* samplePoints = new1<Vector3f>(len);
    int i=0;
    for (std::vector<Vector3f>::iterator it = allSamples.begin();
        it != allSamples.end(); it++)
    {
        samplePoints[i] = *it;
        i++;
    }

    // Compute 2D Convex hull.
    ConvexHull3f *pHull = new0 ConvexHull3f(len, samplePoints, 0.001f, false, Query::QT_REAL);
    assertion(pHull->GetDimension() == 2, "Incorrect dimension.\n");

    ConvexHull2f *pHull2 = pHull->GetConvexHull2();
    int numSimplices = pHull2->GetNumSimplices();
    const int* indices = pHull2->GetIndices();
    Vector3f* ctrlPoints = new1<Vector3f>(numSimplices);
    for (i = 0; i < numSimplices; i++)
    {
        ctrlPoints[i] = samplePoints[indices[i]];
    }

    delete0(pHull);
    delete0(pHull2);
    delete1(samplePoints);

    BSplineCurve3f *pSpline = new0 BSplineCurve3f(numSimplices, ctrlPoints, 2, true, false);
    return pSpline;
}

void TridCircle::TessellateCircle(const Circle2f& cir, int sampleNum, std::vector<Vector3f> &tess)
{
    // Start tessellate arc.
    Transform xform;
    xform.SetTranslate(Vector3f(cir.Center.X(), cir.Center.Y(), 0));

    float angle = Mathf::TWO_PI / sampleNum;
    for (int i = 0; i < sampleNum; ++i)
    {
        float x = cir.Radius * Mathf::Cos(angle * i);
        float y = cir.Radius * Mathf::Sin(angle * i);

        Vector3f position = xform * APoint(x, y, 0);
        AddUniquePoint(position, tess);
    }
}

