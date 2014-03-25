// Geometric Tools, LLC
// Copyright (c) 1998-2013
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2012/07/07)

#include "tbapplication.h"
#include "Wm5ConvexHull3.h"
#include "tbmeshboolean.h"

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
    mBeginTridCircles[2] = Circle3f(Vector3f(3, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.5);
    mEndTridCircles[0] = Circle3f(Vector3f(-1, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.1);
    mEndTridCircles[1] = Circle3f(Vector3f(0, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.2);
    mEndTridCircles[2] = Circle3f(Vector3f(1, 0, 10), Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1), 0.1);
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

    // Shading effect.
    Material* steel = new0 Material();
    steel->Emissive = Float4(0.0f, 0.0f, 0.0f, 1.0f);
    steel->Ambient = Float4(0.24725f, 0.2245f, 0.2645f, 1.0f);
    steel->Diffuse = Float4(0.34615f, 0.3143f, 0.2903f, 1.0f);
    steel->Specular = Float4(0.697357f, 0.623991f, 0.608006f, 83.2f);

    Light *light = new0 Light(Light::LT_DIRECTIONAL);
    light->Ambient = Float4(0.75f, 0.75f, 0.75f, 1.0f);
    light->Diffuse = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    light->Specular = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    light->SetDirection(AVector(1.0f, 1.0f, 1.0f));

    LightDirPerVerEffect* effectDV = new0 LightDirPerVerEffect();
    mEffect = effectDV->CreateInstance(light, steel);

    // Create mesh.
    mScene->AttachChild(CreateMesh());
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

void TBApplication::CreateBody(TBMesh &mesh)
{
    // Create bottom faces.
    int sampleCount = 20;
    float harfHeight = 2;
    float radius = 4;
    Vector3f *vertices = new1<Vector3f>(sampleCount * 2 + 2 );
    float angle = Mathf::TWO_PI / sampleCount;
    for (int i = 0; i < sampleCount; ++i)
    {
        float x = radius * Mathf::Cos(angle * i);
        float y = radius * Mathf::Sin(angle * i);
        vertices[i] = Vector3f(x, y, -harfHeight);
        vertices[i + sampleCount] = Vector3f(x, y, harfHeight);
    }

    vertices[sampleCount * 2] = Vector3f(0, 0, -harfHeight - 0.01);
    vertices[sampleCount * 2 + 1] = Vector3f(0, 0, harfHeight + 0.01);

    ConvexHull3f *pHull = new0 ConvexHull3f(sampleCount * 2 + 2, vertices, 0.0001f, false, Query::QT_REAL);

    int numTriangles = pHull->GetNumSimplices();
    const int* hullIndices = pHull->GetIndices();
    for (int i=0; i<numTriangles; i++) {
        int p1 = hullIndices[i*3];
        int p2 = hullIndices[i*3 + 1];
        int p3 = hullIndices[i*3 + 2];

        mesh.addTriangle(vertices[p1], vertices[p2], vertices[p3]);
    }

    delete1(vertices);
    delete0(pHull);
}

void TBApplication::CreateWing(TBMesh &mesh)
{
    std::vector<Vector3f> samples;
    std::vector<Vector3f> delaunaySamples;
    int sampleCount = 20;
    int delaunaySamplesCount = sampleCount * 2;

    // Create faces.
    CreateSamples(mBeginTridCircles[0], mBeginTridCircles[1], mBeginTridCircles[2], samples, 0);
    for (int step = 1; step < mInterpoStep+1; step++) {

        float height = step * (mHeight * (1.0 / mInterpoStep));

        Circle3f cir1 = LinearCircleInterpolate(mBeginTridCircles[0], mEndTridCircles[0], mInterpoStep, step);
        Circle3f cir2 = LinearCircleInterpolate(mBeginTridCircles[1], mEndTridCircles[1], mInterpoStep, step);
        Circle3f cir3 = LinearCircleInterpolate(mBeginTridCircles[2], mEndTridCircles[2], mInterpoStep, step);

        delaunaySamples.clear();
        delaunaySamples.insert(delaunaySamples.end(), samples.begin(), samples.end());
        samples.clear();
        CreateSamples(cir1, cir2, cir3, samples, height);
        delaunaySamples.insert(delaunaySamples.end(), samples.begin(), samples.end());

        Vector3f *vertices = new1<Vector3f>(sampleCount * 2);
        int i = 0;
        for (std::vector<Vector3f>::iterator it = delaunaySamples.begin();
            it != delaunaySamples.end(); it++, i++) {
            Vector3f pos = *it;
            vertices[i] = pos;
        }
        ConvexHull3f *pHull = new0 ConvexHull3f(sampleCount * 2, vertices, 0.0001f, false, Query::QT_REAL);

        int numTriangles = pHull->GetNumSimplices();
        const int* hullIndices = pHull->GetIndices();
        for (int i=0; i<numTriangles; i++) {

            int p1 = hullIndices[i*3];
            int p2 = hullIndices[i*3 + 1];
            int p3 = hullIndices[i*3 + 2];

            bool isFaceOnTop = p1 >= sampleCount && p2 >= sampleCount && p3 >= sampleCount;
            bool isFaceOnBottom = p1 < sampleCount && p2 < sampleCount && p3 < sampleCount;
            if (isFaceOnTop && step != mInterpoStep) {
                continue;
            }
            if (isFaceOnBottom && step != 1) {
                continue;
            }
            mesh.addTriangle(vertices[p1], vertices[p2], vertices[p3]);
        }

        delete0(pHull);
        delete1(vertices);
    }
}

TriMesh* TBApplication::CreateTriMesh(const TBMesh &mesh) {

    std::vector<Vector3f> vertices;
    std::vector<int> indices;
    std::vector<Vector3f> normals;
    ComputeNormals(mesh, vertices, indices, normals);

    // Create TriMesh for rendering. The normals are duplicated to texture
    // coordinates to avoid the AMD lighting problems due to use of
    // pre-OpenGL2.x extensions.
    VertexFormat* vformat = VertexFormat::Create(3,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_NORMAL, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT3, 1);

    int vstride = vformat->GetStride();
    VertexBuffer* vbuffer = new0 VertexBuffer(vertices.size(), vstride);
    VertexBufferAccessor vba(vformat, vbuffer);
    Float4 lightGray(0.75f, 0.75f, 0.75f, 1.0f);
    std::vector<Vector3f>::const_iterator it =  vertices.begin();
    for (int j=0; it != vertices.end(); it++, j++) {
        vba.Position<Vector3f>(j) = *it;
        Vector3f &normal = normals[j];
        vba.Normal<Vector3f>(j) = normal;
        vba.TCoord<Vector3f>(1, j) = normal;
    }

    int indexCount = indices.size();
    IndexBuffer* ibuffer = new0 IndexBuffer(indexCount, sizeof(int));
    int* indicesBuf = (int*)ibuffer->GetData();

    std::vector<int>::const_iterator iit = indices.begin();
    for (int j=0; iit != indices.end(); iit++, j++) {
        indicesBuf[j] = *iit; 
    }

    TriMesh* tetra = new0 TriMesh(vformat, vbuffer, ibuffer);
    tetra->SetEffectInstance(mEffect);

    return tetra;
}

TriMesh* TBApplication::CreateMesh()
{
    // Create Wings.
    TBMesh *wing1 = new0 TBMesh();
    CreateWing(*wing1);
    Transform rotate;
    rotate.SetRotate(HMatrix(AVector::UNIT_Z, 25.0 * Mathf::PI / 180.0));
    wing1->transformBy(rotate);
    Transform trans;
    trans.SetTranslate(Vector3f(-0.5, 0.0, 2.5));
    wing1->transformBy(trans);

    TBMesh *wing2 = wing1->clone();
    Transform rotate2;
    rotate2.SetRotate(HMatrix(AVector::UNIT_Y, 120.0 * Mathf::PI / 180.0));
    wing2->transformBy(rotate2);

    TBMesh *wing3 = wing1->clone();
    Transform rotate3;
    rotate3.SetRotate(HMatrix(AVector::UNIT_Y, -120.0 * Mathf::PI / 180.0));
    wing3->transformBy(rotate3);

    TBMesh *body = new0 TBMesh();
    CreateBody(*body);
    Transform xform;
    xform.SetRotate(HMatrix(AVector::UNIT_X, Mathf::PI / 2.0));
    body->transformBy(xform);

    // Boolean wings and body.
    TBMesh *result1 = new0 TBMesh();
    TBBoolean::add(*wing1, *body, *result1);
    TBMesh *result2 = new0 TBMesh();
    TBBoolean::add(*wing2, *result1, *result2);
    TBMesh *result3 = new0 TBMesh();
    TBBoolean::add(*wing3, *result2, *result3);

    TriMesh* tMesh = CreateTriMesh(*result3);

    delete0(wing1);
    delete0(wing2);
    delete0(wing3);
    delete0(body);
    delete0(result1);
    delete0(result2);
    delete0(result3);

    return tMesh;
}

void TBApplication::ComputeNormals (const TBMesh &mesh, std::vector<Vector3f> &flatVertices, std::vector<int> &flatIndices, std::vector<Vector3f> &normals)
{
    const std::vector<Vector3f>& vertices = mesh.getVertices();
    const std::vector<int>& indices = mesh.getIndices();

    normals.clear();
    flatVertices.clear();
    flatIndices.clear();

    for (int j=0; j<indices.size(); j+=3) {
        int i1 = indices[j];
        int i2 = indices[j+1];
        int i3 = indices[j+2];
        Vector3f p1 = vertices[i1];
        Vector3f p2 = vertices[i2];
        Vector3f p3 = vertices[i3];

        Vector3f cross1 = p2 - p1;
        Vector3f cross2 = p3 - p1;
        Vector3f normal = cross1.Cross(cross2);
        normal.Normalize();

        // Fill in data.
        flatVertices.push_back(p1);
        flatVertices.push_back(p2);
        flatVertices.push_back(p3);
        flatIndices.push_back(j);
        flatIndices.push_back(j+1);
        flatIndices.push_back(j+2);
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
    }
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
