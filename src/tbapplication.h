
#ifndef TBAPPLICATION_H
#define TBAPPLICATION_H

#include "Wm5WindowApplication3.h"
#include "tridcircle.h"
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
    void CreateScene ();
    void CreateCricleSamples ();
    void CreateBottomTridSamples (int sampleNum);
    void CreateTopSplineSamples (int sampleNum);
    void LoadData ();

    TriMesh* CreateSphere (const Vector3f& origin, float radius);
    TriMesh* CreateTetra (int index);

    int mNumVertices;
    Vector3f* mVertices;

    // A visual representation of the hull.
    NodePtr mScene, mTrnNode;
    WireStatePtr mWireState;
    CullStatePtr mCullState;
    Culler mCuller;

    Delaunay3f* mDelaunay;
    TridCircle* mTridCircle;
};

WM5_REGISTER_INITIALIZE(TBApplication);
WM5_REGISTER_TERMINATE(TBApplication);

#endif
