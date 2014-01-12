
#ifndef TRIDECIRCLE_H
#define TRIDECIRCLE_H

#include "Wm5Core.h"
#include "Wm5Mathematics.h"

using namespace Wm5;

// Only work for trid on xy plane. All the positions should be on the xy plane.
class TridCircle
{
public:
    TridCircle(const Circle3f& circle1, const Circle3f& circle2, const Circle3f& circle3);
    ~TridCircle();

    // The circles for trid.
    void SetCircle1(const Circle3f& circle);
    void SetCircle2(const Circle3f& circle);
    void SetCircle3(const Circle3f& circle);
    Circle3f GetCircle(int index);

    // Create circular spline which is a convex hull of the three trid circles.
    // It is caller's responsibility to clean up memory.
    BSplineCurve3f *CreateCircle();

    void TessCircle(Vector3f tess[]);

private:
    bool GetTangentPosition(const Circle2f& circle, const Line2f& line2, Vector3f& point);

    Line2f GetOuterLine(Line2f line[4], const Vector2f& fromPos);

    void TessellateArc(const Vector3f& startPos,
                       const Vector3f& endPos,
                       const Vector3f& center,
                       int sampleNum,
                       std::vector<Vector3f> &tess);

    // Convert between Circle3f and Circle2f.
    Circle2f ToCircle2f(const Circle3f& circle3);
    Circle3f ToCircle3f(const Circle2f& circle2);

    bool ValidatePosition(const Vector3f& center,
                          const Vector3f& normal,
                          Vector3f& startPoint,
                          Vector3f& endPoint);

private:
    Circle3f m_Circles[3];
};

#endif 