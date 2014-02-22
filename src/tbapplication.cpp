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
}

TBApplication::~TBApplication ()
{
}
//----------------------------------------------------------------------------

void TBApplication::InitializeDataModel ()
{
    mBeginTridCircles[0] = Circle3f(Vector3f(-2, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
    mBeginTridCircles[1] = Circle3f(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 1.0);
    mBeginTridCircles[2] = Circle3f(Vector3f(4, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
    mEndTridCircles[0] = Circle3f(Vector3f(-1, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.1);
    mEndTridCircles[1] = Circle3f(Vector3f(0, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.11);
    mEndTridCircles[2] = Circle3f(Vector3f(2, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.1);
    mInterpoStep = 10;
    mHeight = 10;
}

bool TBApplication::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

    InitializeDataModel();

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

    // Create mesh.
    TriMesh* mesh = CreateMesh();
    mScene->AttachChild(mesh);
}

void TBApplication::CreateSamples(const Circle3f& circle1,
                                  const Circle3f& circle2,
                                  const Circle3f& circle3,
                                  std::vector<Vector3f>& vertices,
                                  float height) {

    TridCircle *tc = new0 TridCircle(circle1, circle2, circle3);
    BSplineCurve3f *pSpline = tc->CreateCircle();

    float mult = 1.0f/20;
    int i = 0;
    for (; i < 20; ++i)
    {
        Vector3f pos = pSpline->GetPosition(mult * i);
        // TridCircle always compute the circle on xy plane.
        pos.Z() = height;
        vertices.push_back(pos);
    }
    delete0(tc);
    delete0(pSpline);
}

Circle3f TBApplication::LinearCircleInterpolate(const Circle3f& circleBegin, const Circle3f& circleEnd,
                                                int count, int index) {
    // Compute center position.
    Vector3f dir = circleEnd.Center - circleBegin.Center;
    float distance = dir.Normalize();
    Vector3f center = circleBegin.Center + dir * (distance * (index * 1.0 / count));

    float radius = circleBegin.Radius + (circleEnd.Radius - circleBegin.Radius) * (index * 1.0 / count);
    return Circle3f(center, circleBegin.Direction0, circleBegin.Direction1, circleBegin.Normal, radius);
}

TriMesh* TBApplication::CreateMesh() {

    std::vector<Vector3f> allVertices;
    std::vector<Vector3f> samples;
    std::vector<Vector3f> delaunaySamples;
    std::vector<int> indices;
    int currentIndex = 0;
    int sampleCount = 20;
    int delaunaySamplesCount = sampleCount * 2;

    // Create silhouette mesh.
    for (int step = 0; step < mInterpoStep+1; step++) {

        float height = step * (mHeight * (1.0 / mInterpoStep));

        Circle3f cir1 = LinearCircleInterpolate(mBeginTridCircles[0], mEndTridCircles[0], mInterpoStep, step);
        Circle3f cir2 = LinearCircleInterpolate(mBeginTridCircles[1], mEndTridCircles[1], mInterpoStep, step);
        Circle3f cir3 = LinearCircleInterpolate(mBeginTridCircles[2], mEndTridCircles[2], mInterpoStep, step);

        delaunaySamples.clear();
        delaunaySamples.insert(delaunaySamples.end(), samples.begin(), samples.end());
        samples.clear();
        CreateSamples(cir1, cir2, cir3, samples, height);
        delaunaySamples.insert(delaunaySamples.end(), samples.begin(), samples.end());

        if (delaunaySamples.size() == delaunaySamplesCount) {
            Vector3f *vertices = new1<Vector3f>(sampleCount * 2);
            int i = 0;
            for (std::vector<Vector3f>::iterator it = delaunaySamples.begin();
                it != delaunaySamples.end(); it++, i++) {
                Vector3f pos = *it;
                vertices[i] = pos;
            }
            ConvexHull3f *pHull = new0 ConvexHull3f(sampleCount * 2, vertices, 0.0001f, false, Query::QT_REAL);
            
            int indexOffset = allVertices.size() - sampleCount;

            int numTriangles = pHull->GetNumSimplices();
            const int* hullIndices = pHull->GetIndices();
            for (int i=0; i<numTriangles; i++) {

                int p1 = hullIndices[i*3];
                int p2 = hullIndices[i*3 + 1];
                int p3 = hullIndices[i*3 + 2];

                if ((p1 < sampleCount && p2 < sampleCount && p3 < sampleCount) || 
                    (p1 >= sampleCount && p2 >= sampleCount && p3 >= sampleCount)) {
                    continue;
                }

                indices.push_back(p1 + indexOffset);
                indices.push_back(p2 + indexOffset);
                indices.push_back(p3 + indexOffset);
            }

            delete0(pHull);
            delete1(vertices);
        }

        // Adding sample vertices.
        allVertices.insert(allVertices.end(), samples.begin(), samples.end());
        currentIndex += sampleCount;
    }

    // Adding top and bottom faces.
    allVertices.push_back(mBeginTridCircles[1].Center);
    int btCenter = allVertices.size() - 1;
    for (int i=0; i<sampleCount; i++) {
        indices.push_back(i);
        indices.push_back(btCenter);
        int p2 = i + 1;
        if (p2 == sampleCount) {
            p2 = 0;
        }
        indices.push_back(p2);
    }

    allVertices.push_back(mEndTridCircles[1].Center);
    int tpCenter = allVertices.size() - 1;
    for (int i=currentIndex-sampleCount; i<currentIndex; i++) {
        indices.push_back(i);
        indices.push_back(tpCenter);
        int p2 = i + 1;
        if (p2 == currentIndex) {
            p2 = currentIndex-sampleCount;
        }
        indices.push_back(p2);
    }

    // Create mesh.
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_COLOR, VertexFormat::AT_FLOAT4, 0);
    int vstride = vformat->GetStride();

    VertexBuffer* vbuffer = new0 VertexBuffer(allVertices.size(), vstride);
    VertexBufferAccessor vba(vformat, vbuffer);
    Float4 lightGray(0.75f, 0.75f, 0.75f, 1.0f);
    std::vector<Vector3f>::iterator it =  allVertices.begin();
    for (int j=0; it != allVertices.end(); it++, j++) {
        vba.Position<Vector3f>(j) = *it;
        vba.Color<Float4>(0, j) = lightGray;
    }

    int indexCount = indices.size();
    IndexBuffer* ibuffer = new0 IndexBuffer(indexCount, sizeof(int));
    int* indicesBuf = (int*)ibuffer->GetData();

    std::vector<int>::iterator iit =  indices.begin();
    for (int j=0; iit != indices.end(); iit++, j++) {
        indicesBuf[j] = *iit; 
    }

    TriMesh* tetra = new0 TriMesh(vformat, vbuffer, ibuffer);
    VisualEffectInstance* instance = VertexColor4Effect::CreateUniqueInstance();
    instance->GetEffect()->GetAlphaState(0, 0)->BlendEnabled = true;
    instance->GetEffect()->GetWireState(0, 0)->Enabled = true;
    tetra->SetEffectInstance(instance);

    return tetra;
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
