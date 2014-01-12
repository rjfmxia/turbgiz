

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

Line2<float> TridCircle::GetOuterLine(Line2f line[4], const Vector2f& fromPos)
{
    Line2f outerLine = line[0];
    DistPoint2Line2f queryPL(fromPos, line[0]);
    float maxDis = queryPL.GetSquared();
    for (int i=1; i<4; i++)
    {
        DistPoint2Line2f queryPL(fromPos, line[i]);
        float dist = queryPL.GetSquared();
        if (dist > maxDis)
        {
            outerLine = line[i];
            maxDis = dist;
        }
    }
    return outerLine;
}

bool TridCircle::GetTangentPosition(const Circle2f& circle,
                                    const Line2f& line2,
                                    Vector3f& point)
{
    // Have no DistLine2Circle2...
    Circle3f cir = ToCircle3f(circle);

    Vector3f lOri(line2.Origin.X(), line2.Origin.Y(), 0);
    Vector3f lDir(line2.Direction.X(), line2.Direction.Y(), 0);
    Line3f line3(lOri, lDir);

    DistLine3Circle3f cl(line3, cir);
    cl.GetSquared();
    Vector3f p = cl.GetClosestPoint0();
    point.X() = p.X();
    point.Y() = p.Y();
    point.Z() = p.Z();
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

bool TridCircle::ValidatePosition(const Vector3f& center, const Vector3f& normal, Vector3f& startPoint, Vector3f& endPoint)
{
    AVector startDir = APoint(startPoint) - APoint(center);
    AVector endDir = APoint(endPoint) - APoint(center);

    if (startDir.Cross(endDir).Dot(normal) < 0)
    {
        Vector3f temp = startPoint;
        startPoint = endPoint;
        endPoint = temp;
    }
}

void TridCircle::TessCircle(Vector3f tessPositions[])
{
    Circle2f cir0 = ToCircle2f(m_Circles[0]);
    Circle2f cir1 = ToCircle2f(m_Circles[1]);
    Circle2f cir2 = ToCircle2f(m_Circles[2]);

    // Adding line samples.
    Vector3f cir0Start, cir0End, cir1Start, cir1End, cir2Start, cir2End;
    Line2f lines[4];
    if (GetTangentsToCircles(cir0, cir1, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir2.Center);
        GetTangentPosition(cir0, outerLine, cir0Start);
        GetTangentPosition(cir1, outerLine, cir1Start);
    }

    if (GetTangentsToCircles(cir0, cir2, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir1.Center);
        GetTangentPosition(cir0, outerLine, cir0End);
        GetTangentPosition(cir2, outerLine, cir2Start);
    }

    if (GetTangentsToCircles(cir1, cir2, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir0.Center);
        GetTangentPosition(cir1, outerLine, cir1End);
        GetTangentPosition(cir2, outerLine, cir2End);
    }

    Vector3f normal(0, 0, 1);
    ValidatePosition(m_Circles[0].Center, normal, cir0Start, cir0End);
    ValidatePosition(m_Circles[1].Center, normal, cir1Start, cir1End);
    ValidatePosition(m_Circles[2].Center, normal, cir2Start, cir2End);

    // Adding samples.
    int sampleNum = 10;
    std::vector<Vector3f> tess;
    TessellateArc(cir0Start, cir0End, m_Circles[0].Center, sampleNum, tess);
    TessellateArc(cir1Start, cir1End, m_Circles[1].Center, sampleNum, tess);
    TessellateArc(cir2Start, cir2End, m_Circles[2].Center, sampleNum, tess);

    // Vector to array.
    int len = tess.size();
    Vector3f* ctrlPoints = new1<Vector3f>(len);
    int j=0;
    for (std::vector<Vector3f>::iterator it = tess.begin();
        it != tess.end(); it++)
    {
        tessPositions[j] = *it;
        j++;
    }
}

BSplineCurve3f *TridCircle::CreateCircle()
{
    Circle2f cir0 = ToCircle2f(m_Circles[0]);
    Circle2f cir1 = ToCircle2f(m_Circles[1]);
    Circle2f cir2 = ToCircle2f(m_Circles[2]);

    // Adding line samples.
    Vector3f cir0Start, cir0End, cir1Start, cir1End, cir2Start, cir2End;
    Line2f lines[4];
    if (GetTangentsToCircles(cir0, cir1, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir2.Center);
        GetTangentPosition(cir0, outerLine, cir0Start);
        GetTangentPosition(cir1, outerLine, cir1Start);
    }

    if (GetTangentsToCircles(cir0, cir2, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir1.Center);
        GetTangentPosition(cir0, outerLine, cir0End);
        GetTangentPosition(cir2, outerLine, cir2Start);
    }

    if (GetTangentsToCircles(cir1, cir2, lines))
    {
        Line2f outerLine = GetOuterLine(lines, cir0.Center);
        GetTangentPosition(cir1, outerLine, cir1End);
        GetTangentPosition(cir2, outerLine, cir2End);
    }

    Vector3f normal(0, 0, 1);
    ValidatePosition(m_Circles[0].Center, normal, cir0Start, cir0End);
    ValidatePosition(m_Circles[1].Center, normal, cir1Start, cir1End);
    ValidatePosition(m_Circles[2].Center, normal, cir2Start, cir2End);

    // Adding samples.
    int sampleNum = 10;
    std::vector<Vector3f> tess;
    TessellateArc(cir0Start, cir0End, m_Circles[0].Center, sampleNum, tess);
    TessellateArc(cir1Start, cir1End, m_Circles[1].Center, sampleNum, tess);
    TessellateArc(cir2Start, cir2End, m_Circles[2].Center, sampleNum, tess);

    // Vector to array.
    int len = tess.size();
    Vector3f* ctrlPoints = new1<Vector3f>(len);
    int j=0;
    for (std::vector<Vector3f>::iterator it = tess.begin();
        it != tess.end(); it++)
    {
        ctrlPoints[j] = *it;
        j++;
    }

    BSplineCurve3f *pSpline = new0 BSplineCurve3f(len, ctrlPoints, 2, true, false);
    return pSpline;
}

void TridCircle::TessellateArc(const Vector3f& startPos,
                               const Vector3f& endPos,
                               const Vector3f& center,
                               int sampleNum,
                               std::vector<Vector3f> &tess)
{
    // Compute angle.
    AVector startDir = APoint(startPos) - APoint(center);
    AVector endDir = APoint(endPos) - APoint(center);
    startDir.Normalize();
    endDir.Normalize();

    float dot = startDir.Dot(endDir);
    float angle = Mathf::ACos(dot);
    float step = angle / (sampleNum - 2);

    // Start tessellate arc.
    Transform trans;
    trans.SetTranslate(center);
    Transform transBack;
    transBack.SetTranslate(-center);

    tess.push_back(startPos);
    for (int i = 1; i < sampleNum - 1; i++)
    {
        Transform rotate;
        rotate.SetRotate(HMatrix(AVector::UNIT_Z, i*step));

        Transform xform = trans * rotate * transBack;
        Vector3f position = xform * APoint(startPos);
        tess.push_back(position);
    }
    tess.push_back(endPos);
}

