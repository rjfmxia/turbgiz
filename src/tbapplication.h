
#ifndef TBAPPLICATION_H
#define TBAPPLICATION_H

#include "Wm5WindowApplication3.h"
#include "tridcircle.h"
#include "tbmesh.h"

using namespace Wm5;

class TBApplication : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    TBApplication ();
    ~TBApplication ();

    virtual bool OnInitialize ();
    virtual void OnTerminate ();
    virtual void OnIdle ();
    virtual bool OnKeyDown (unsigned char key, int x, int y);

protected:
    void InitializeDataModel();

    Circle3f LinearCircleInterpolate(const Circle3f& circle1,
                                     const Circle3f& circle2,
                                     int count, int index);

    void CreateSamples(const Circle3f& circle1,
                       const Circle3f& circle2,
                       const Circle3f& circle3,
                       std::vector<Vector3f>& vertices,
                       float height);

    // Mesh create methods.
    void CreateWing(TBMesh &mesh);
    void CreateBody(TBMesh &mesh);
    TriMesh* CreateMesh();
    TriMesh* CreateTriMesh(TBMesh &mesh);

    void CreateScene ();
    TriMesh* CreateSphere (const Vector3f& origin, float radius);

    // A visual representation of the hull.
    NodePtr mScene, mTrnNode;
    WireStatePtr mWireState;
    CullStatePtr mCullState;
    Culler mCuller;

    Circle3f mBeginTridCircles[3];
    Circle3f mEndTridCircles[3];
    int mInterpoStep;
    int mHeight;
};

WM5_REGISTER_INITIALIZE(TBApplication);
WM5_REGISTER_TERMINATE(TBApplication);

#endif
