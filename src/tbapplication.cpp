// Geometric Tools, LLC
// Copyright (c) 1998-2013
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2012/07/07)

#include "tbapplication.h"
#include "Wm5ConvexHull3.h"

WM5_WINDOW_APPLICATION(TBApplication);

//----------------------------------------------------------------------------
TBApplication::TBApplication ()
    :
    WindowApplication3("SampleMathematics/TBApplication", 0, 0, 640, 480,
        Float4(1.0f, 1.0f, 1.0f, 1.0f))
{
    Environment::InsertDirectory(ThePath + "Data/");

    mNumVertices = 0;
    mVertices = 0;
    mDelaunay = 0;
    mTridCircle = 0;
}

TBApplication::~TBApplication ()
{
    delete0(mTridCircle);
}
//----------------------------------------------------------------------------
bool TBApplication::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

    // The scene creation involves culling, so mCuller needs to know its
    // camera now.
    mCuller.SetCamera(mCamera);
    CreateScene();

    // Center-and-fit for camera viewing.
    mScene->Update();
    mTrnNode->LocalTransform.SetTranslate(-mScene->WorldBound.GetCenter());
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 10000.0f);
    AVector camDVector(0.0f, 0.0f, 1.0f);
    AVector camUVector(0.0f, 1.0f, 0.0f);
    AVector camRVector = camDVector.Cross(camUVector);
    APoint camPosition = APoint::ORIGIN -
        2.5f*mScene->WorldBound.GetRadius()*camDVector;
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    // Initial update of objects.
    mScene->Update();

    // Initial culling of scene.
    mCuller.ComputeVisibleSet(mScene);

    InitializeCameraMotion(1.0f, 0.01f);
    InitializeObjectMotion(mScene);
    return true;
}
//----------------------------------------------------------------------------
void TBApplication::OnTerminate ()
{
    delete1(mVertices);
    delete0(mTridCircle);
    delete0(mDelaunay);

    mScene = 0;
    mWireState = 0;
    mCullState = 0;
    WindowApplication3::OnTerminate();
}
//----------------------------------------------------------------------------
void TBApplication::OnIdle ()
{
    MeasureTime();

    if (MoveCamera())
    {
        mCuller.ComputeVisibleSet(mScene);
    }

    if (MoveObject())
    {
        mScene->Update();
        mCuller.ComputeVisibleSet(mScene);
    }

    if (mRenderer->PreDraw())
    {
        mRenderer->ClearBuffers();
        mRenderer->Draw(mCuller.GetVisibleSet());
        mRenderer->PostDraw();
        mRenderer->DisplayColorBuffer();
    }

    UpdateFrameCount();
}
//----------------------------------------------------------------------------
bool TBApplication::OnKeyDown (unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        mWireState->Enabled = !mWireState->Enabled;
        return true;
    }

    return WindowApplication::OnKeyDown(key, x, y);
}

void TBApplication::CreateTopSplineSamples(int sampleNum)
{
   //  Vector3f center1(-2, 0, 0);
   //  Vector3f center2(2, 0, 0);
   //  Vector3f center3(0, 2, 0);
   //  Circle3f cir1(center1, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
   //  Circle3f cir2(center2, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
   //  Circle3f cir3(center3, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
   //  mTridCircle = new0 TridCircle(cir1, cir2, cir3);

   //  BSplineCurve3f *pSpline = mTridCircle->CreateCircle();

   //  int lineSegPoints = 4;
   //  mNumVertices = sampleNum + lineSegPoints;
   //  mVertices = new1<Vector3f>(mNumVertices);
   //  float mult = 1.0f/(sampleNum - 1);
   //  int i = 0;
   //  for (; i < sampleNum; ++i)
   //  {
   //      mVertices[i] = pSpline->GetPosition(mult * i);
   //  }

   //  mVertices[i++] = Vector3f(0, 0.25, 5);
   //  mVertices[i++] = Vector3f(0, 0, 5);
   //  mVertices[i++] = Vector3f(0, -0.25, 5);
   //  mVertices[i++] = Vector3f(0, -0.5, 5);
   // // mVertices[i++] = Vector3f(0, 0, 0);
   //  delete0(pSpline);
}

void TBApplication::CreateBottomTridSamples (int sampleNum)
{
    Vector3f center1(-2, 0, 0);
    Vector3f center2(0, 0, 0);
    Vector3f center3(4, 0, 0);
    Circle3f cir1(center1, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
    Circle3f cir2(center2, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 1.0);
    Circle3f cir3(center3, Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.2);
    mTridCircle = new0 TridCircle(cir1, cir2, cir3);

    BSplineCurve3f *pSpline = mTridCircle->CreateCircle();

    int lineSegPoints = 4;
    mNumVertices = sampleNum + lineSegPoints;
    mVertices = new1<Vector3f>(mNumVertices);
    float mult = 1.0f/(sampleNum - 1);
    int i = 0;
    for (; i < sampleNum; ++i)
    {
        mVertices[i] = pSpline->GetPosition(mult * i);
    }

    mVertices[i++] = Vector3f(0, 0.25, 5);
    mVertices[i++] = Vector3f(0, 0, 5);
    mVertices[i++] = Vector3f(0, -0.25, 5);
    mVertices[i++] = Vector3f(0, -0.5, 5);
   // mVertices[i++] = Vector3f(0, 0, 0);
    delete0(pSpline);
}

void TBApplication::CreateCricleSamples ()
{
    mNumVertices = 20;
    mVertices = new1<Vector3f>(mNumVertices);

    float angle = Mathf::TWO_PI / (mNumVertices / 2);
    int i;
    for (i = 0; i < mNumVertices / 2; ++i)
    {
        mVertices[i].X() = Mathf::Cos(angle * i);
        mVertices[i].Y() = Mathf::Sin(angle * i);
        mVertices[i].Z() = 0;
    }

    for (; i < mNumVertices; ++i)
    {
        mVertices[i].X() = 0.5 * Mathf::Cos(angle * i);
        mVertices[i].Y() = 0.5 * Mathf::Sin(angle * i);
        mVertices[i].Z() = 1;
    }
}

TriMesh* TBApplication::CreateTetra (int index)
{
    const Vector3f* dvertices = mDelaunay->GetVertices();
    const int* dindices = mDelaunay->GetIndices();

    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_COLOR, VertexFormat::AT_FLOAT4, 0);
    int vstride = vformat->GetStride();

    VertexBuffer* vbuffer = new0 VertexBuffer(4, vstride);
    VertexBufferAccessor vba(vformat, vbuffer);
    vba.Position<Vector3f>(0) = dvertices[dindices[4*index    ]];
    vba.Position<Vector3f>(1) = dvertices[dindices[4*index + 1]];
    vba.Position<Vector3f>(2) = dvertices[dindices[4*index + 2]];
    vba.Position<Vector3f>(3) = dvertices[dindices[4*index + 3]];
    Float4 lightGray(0.75f, 0.75f, 0.75f, 1.0f);
    vba.Color<Float4>(0, 0) = lightGray;
    vba.Color<Float4>(0, 1) = lightGray;
    vba.Color<Float4>(0, 2) = lightGray;
    vba.Color<Float4>(0, 3) = lightGray;

    IndexBuffer* ibuffer = new0 IndexBuffer(12, sizeof(int));
    int* indices = (int*)ibuffer->GetData();
    indices[ 0] = 0;  indices[ 1] = 1;  indices[ 2] = 2;
    indices[ 3] = 0;  indices[ 4] = 3;  indices[ 5] = 1;
    indices[ 6] = 0;  indices[ 7] = 2;  indices[ 8] = 3;
    indices[ 9] = 3;  indices[10] = 2;  indices[11] = 1;

    TriMesh* tetra = new0 TriMesh(vformat, vbuffer, ibuffer);
    VisualEffectInstance* instance =
        VertexColor4Effect::CreateUniqueInstance();
    instance->GetEffect()->GetAlphaState(0, 0)->BlendEnabled = true;
    instance->GetEffect()->GetWireState(0, 0)->Enabled = true;
    tetra->SetEffectInstance(instance);

    return tetra;
}

//----------------------------------------------------------------------------
void TBApplication::CreateScene ()
{
    mScene = new0 Node();
    mTrnNode = new0 Node();
    mScene->AttachChild(mTrnNode);
    mWireState = new0 WireState();
    mRenderer->SetOverrideWireState(mWireState);
    mCullState = new0 CullState();
    mCullState->Enabled = false;
    mRenderer->SetOverrideCullState(mCullState);

    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_COLOR, VertexFormat::AT_FLOAT3, 0);

    TriMesh* sphere = StandardMesh(vformat).Sphere(8, 8, 0.01f);
    sphere->SetEffectInstance(VertexColor3Effect::CreateUniqueInstance());
    mTrnNode->SetChild(1, sphere);

    LoadData();
}

void TBApplication::LoadData ()
{
    delete1(mVertices);

    CreateBottomTridSamples(20);

    mDelaunay = new0 Delaunay3f(mNumVertices, mVertices, 0.001f, true,
        Query::QT_REAL);

    for (int j = 0; j < mDelaunay->GetNumSimplices(); ++j)
    {
        mScene->AttachChild(CreateTetra(j));
    }

    // Create three sphere as indicators.
    Circle3f cir0 = mTridCircle->GetCircle(0);
    TriMesh* sphere0 = CreateSphere(cir0.Center, cir0.Radius);
    mScene->AttachChild(sphere0);

    Circle3f cir1 = mTridCircle->GetCircle(1);
    TriMesh* sphere1 = CreateSphere(cir1.Center, cir1.Radius);
    mScene->AttachChild(sphere1);

    Circle3f cir2 = mTridCircle->GetCircle(2);
    TriMesh* sphere2 = CreateSphere(cir2.Center, cir2.Radius);
    mScene->AttachChild(sphere2);
}

//----------------------------------------------------------------------------
TriMesh* TBApplication::CreateSphere (const Vector3f& origin, float radius)
{
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_COLOR, VertexFormat::AT_FLOAT3, 0);

    Float3 white(0.0f, 0.2f, 0.8f);
    TriMesh* sphere = StandardMesh(vformat).Sphere(8, 8, radius);
    VertexBufferAccessor vba(sphere);
    for (int i = 0; i < vba.GetNumVertices(); ++i)
    {
        vba.Color<Float3>(0, i) = white;
    }

    sphere->SetEffectInstance(VertexColor3Effect::CreateUniqueInstance());
    sphere->LocalTransform.SetTranslate(origin);
    return sphere;
}
//----------------------------------------------------------------------------
