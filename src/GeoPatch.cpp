// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoPatch.h"

#include "GeoPatchContext.h"
#include "GeoPatchJobs.h"
#include "GeoSphere.h"
#include "MathUtil.h"
#include "Pi.h"
#include "RefCounted.h"
#include "galaxy/SystemBody.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "perlin.h"
#include "profiler/Profiler.h"
#include "vcacheopt/vcacheopt.h"

#include <algorithm>
#include <deque>
#include <sstream>

#define DEBUG_CENTROIDS 0

#if DEBUG_PATCHES
#ifdef DEBUG_BOUNDING_SPHERES
#include "graphics/RenderState.h"
#endif

#include "graphics/Drawables.h"
#endif

namespace PatchMaths{
	// in patch surface coords, [0,1]
	inline vector3d GetSpherePoint(const vector3d &v0, const vector3d &v1, const vector3d &v2, const vector3d &v3, const double x, const double y)
	{
		return (v0 + x * (1.0 - y) * (v1 - v0) + x * y * (v2 - v0) + (1.0 - x) * y * (v3 - v0)).Normalized();
	}
}

// tri edge lengths
static constexpr double GEOPATCH_SUBDIVIDE_AT_CAMDIST = 5.0;

#if USE_SUB_CENTROID_CLIPPING
#if NUM_HORIZON_POINTS == 9
static const vector2d sample[NUM_HORIZON_POINTS] = {
	{ 0.2, 0.2 }, { 0.5, 0.2 }, { 0.8, 0.2 },
	{ 0.2, 0.5 }, { 0.5, 0.5 }, { 0.8, 0.8 },
	{ 0.2, 0.8 }, { 0.5, 0.8 }, { 0.8, 0.8 }
};
#elif NUM_HORIZON_POINTS == 5
static const vector2d sample[NUM_HORIZON_POINTS] = {
	// Naive ordering
	// { 0.2, 0.2 }, { 0.8, 0.2 }, // top
	//{ 0.5, 0.5 }, // middle
	//{ 0.2, 0.8 }, { 0.8, 0.8 } // bottom
	
	// possibly more optimal elimination for early out
	{ 0.5, 0.5 }, // centre
	{ 0.2, 0.2 }, // top-left corner
	{ 0.8, 0.8 }, // opposite corner
	{ 0.8, 0.2 }, // other diagonal
	{ 0.2, 0.8 }  // ^^^
};
#endif
#endif // #if USE_SUB_CENTROID_CLIPPING

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs,
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
	const int depth, const GeoPatchID &ID_) :
	m_ctx(ctx_),
	m_corners(new Corners(v0_, v1_, v2_, v3_)),
	m_geosphere(gs),
	m_PatchID(ID_),
	m_depth(depth),
	m_needUpdateVBOs(false),
	m_hasJobRequest(false)
{
	m_clipCentroid = ((v0_ + v1_ + v2_ + v3_) * 0.25).Normalized();
	m_clipRadius = 0.0;
	m_clipRadius = std::max(m_clipRadius, (v0_ - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (v1_ - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (v2_ - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (v3_ - m_clipCentroid).Length());

	double distMult;
	if (m_geosphere->GetSystemBody()->GetType() < SystemBody::TYPE_PLANET_ASTEROID) {
		distMult = 10.0 / Clamp(m_depth, 1, 10);
	} else {
		distMult = 5.0 / Clamp(m_depth, 1, 5);
	}
	m_splitLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, m_depth) * distMult;

#if USE_SUB_CENTROID_CLIPPING
	for (size_t i = 0; i < NUM_HORIZON_POINTS; i++) {
		m_clipHorizon[i] = SSphere(PatchMaths::GetSpherePoint(v0_, v1_, v2_, v3_, sample[i].x, sample[i].y), m_clipRadius * CLIP_RADIUS_MULTIPLIER);
	}
#endif // #if USE_SUB_CENTROID_CLIPPING
}

GeoPatch::~GeoPatch()
{
	m_hasJobRequest = false;
	for (int i = 0; i < NUM_KIDS; i++) {
		m_kids[i].reset();
	}
	m_corners.reset();
	m_patchVBOData.reset();
}

void GeoPatch::UpdateVBOs(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	if (m_needUpdateVBOs) {
		assert(renderer);
		m_needUpdateVBOs = false;

		//create buffer and upload data
		auto vbd = Graphics::VertexBufferDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);
		vbd.numVertices = m_ctx->NUMVERTICES();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		Graphics::VertexBuffer *vtxBuffer = renderer->CreateVertexBuffer(vbd);

		GeoPatchContext::VBOVertex *VBOVtxPtr = vtxBuffer->Map<GeoPatchContext::VBOVertex>(Graphics::BUFFER_MAP_WRITE);
		assert(vtxBuffer->GetDesc().stride == sizeof(GeoPatchContext::VBOVertex));

		const Sint32 edgeLen = m_ctx->GetEdgeLen();
		const double frac = m_ctx->GetFrac();
		const double *pHts = m_patchVBOData->m_heights.get();
		const vector3f *pNorm = m_patchVBOData->m_normals.get();
		const Color3ub *pColr = m_patchVBOData->m_colors.get();

		double minh = DBL_MAX;

		const vector3d v0 = m_corners->m_v0;
		const vector3d v1 = m_corners->m_v1;
		const vector3d v2 = m_corners->m_v2;
		const vector3d v3 = m_corners->m_v3;

		// ----------------------------------------------------
		// inner loops
		for (Sint32 y = 1; y < edgeLen - 1; y++) {
			for (Sint32 x = 1; x < edgeLen - 1; x++) {
				const double height = *pHts;
				minh = std::min(height, minh);
				const double xFrac = double(x - 1) * frac;
				const double yFrac = double(y - 1) * frac;
				const vector3d p((PatchMaths::GetSpherePoint(v0, v1, v2, v3, xFrac, yFrac) * (height + 1.0)) - m_clipCentroid);
				m_clipRadius = std::max(m_clipRadius, p.Length());

				GeoPatchContext::VBOVertex *vtxPtr = &VBOVtxPtr[x + (y * edgeLen)];
				vtxPtr->pos = vector3f(p);
				++pHts; // next height

				const vector3f norma(pNorm->Normalized());
				vtxPtr->norm = norma;
				++pNorm; // next normal

				vtxPtr->col[0] = pColr->r;
				vtxPtr->col[1] = pColr->g;
				vtxPtr->col[2] = pColr->b;
				vtxPtr->col[3] = 255;
				++pColr; // next colour

				// uv coords
				vtxPtr->uv.x = 1.0f - xFrac;
				vtxPtr->uv.y = yFrac;

				++vtxPtr; // next vertex
			}
		}
		const double minhScale = (minh + 1.0) * 0.999995;
		// ----------------------------------------------------
		const Sint32 innerLeft = 1;
		const Sint32 innerRight = edgeLen - 2;
		const Sint32 outerLeft = 0;
		const Sint32 outerRight = edgeLen - 1;
		// vertical edges
		// left-edge
		for (Sint32 y = 1; y < edgeLen - 1; y++) {
			const Sint32 x = innerLeft - 1;
			const double xFrac = double(x - 1) * frac;
			const double yFrac = double(y - 1) * frac;
			const vector3d p((PatchMaths::GetSpherePoint(v0, v1, v2, v3, xFrac, yFrac) * minhScale) - m_clipCentroid);

			GeoPatchContext::VBOVertex *vtxPtr = &VBOVtxPtr[outerLeft + (y * edgeLen)];
			GeoPatchContext::VBOVertex *vtxInr = &VBOVtxPtr[innerLeft + (y * edgeLen)];
			vtxPtr->pos = vector3f(p);
			vtxPtr->norm = vtxInr->norm;
			vtxPtr->col = vtxInr->col;
			vtxPtr->uv = vtxInr->uv;
		}
		// right-edge
		for (Sint32 y = 1; y < edgeLen - 1; y++) {
			const Sint32 x = innerRight + 1;
			const double xFrac = double(x - 1) * frac;
			const double yFrac = double(y - 1) * frac;
			const vector3d p((PatchMaths::GetSpherePoint(v0, v1, v2, v3, xFrac, yFrac) * minhScale) - m_clipCentroid);

			GeoPatchContext::VBOVertex *vtxPtr = &VBOVtxPtr[outerRight + (y * edgeLen)];
			GeoPatchContext::VBOVertex *vtxInr = &VBOVtxPtr[innerRight + (y * edgeLen)];
			vtxPtr->pos = vector3f(p);
			vtxPtr->norm = vtxInr->norm;
			vtxPtr->col = vtxInr->col;
			vtxPtr->uv = vtxInr->uv;
		}
		// ----------------------------------------------------
		const Sint32 innerTop = 1;
		const Sint32 innerBottom = edgeLen - 2;
		const Sint32 outerTop = 0;
		const Sint32 outerBottom = edgeLen - 1;
		// horizontal edges
		// top-edge
		for (Sint32 x = 1; x < edgeLen - 1; x++) {
			const Sint32 y = innerTop - 1;
			const double xFrac = double(x - 1) * frac;
			const double yFrac = double(y - 1) * frac;
			const vector3d p((PatchMaths::GetSpherePoint(v0, v1, v2, v3, xFrac, yFrac) * minhScale) - m_clipCentroid);

			GeoPatchContext::VBOVertex *vtxPtr = &VBOVtxPtr[x + (outerTop * edgeLen)];
			GeoPatchContext::VBOVertex *vtxInr = &VBOVtxPtr[x + (innerTop * edgeLen)];
			vtxPtr->pos = vector3f(p);
			vtxPtr->norm = vtxInr->norm;
			vtxPtr->col = vtxInr->col;
			vtxPtr->uv = vtxInr->uv;
		}
		// bottom-edge
		for (Sint32 x = 1; x < edgeLen - 1; x++) {
			const Sint32 y = innerBottom + 1;
			const double xFrac = double(x - 1) * frac;
			const double yFrac = double(y - 1) * frac;
			const vector3d p((PatchMaths::GetSpherePoint(v0, v1, v2, v3, xFrac, yFrac) * minhScale) - m_clipCentroid);

			GeoPatchContext::VBOVertex *vtxPtr = &VBOVtxPtr[x + (outerBottom * edgeLen)];
			GeoPatchContext::VBOVertex *vtxInr = &VBOVtxPtr[x + (innerBottom * edgeLen)];
			vtxPtr->pos = vector3f(p);
			vtxPtr->norm = vtxInr->norm;
			vtxPtr->col = vtxInr->col;
			vtxPtr->uv = vtxInr->uv;
		}
		// ----------------------------------------------------
		// corners
		{
			// top left
			GeoPatchContext::VBOVertex *tarPtr = &VBOVtxPtr[0];
			GeoPatchContext::VBOVertex *srcPtr = &VBOVtxPtr[1];
			(*tarPtr) = (*srcPtr);
		}
		{
			// top right
			GeoPatchContext::VBOVertex *tarPtr = &VBOVtxPtr[(edgeLen - 1)];
			GeoPatchContext::VBOVertex *srcPtr = &VBOVtxPtr[(edgeLen - 2)];
			(*tarPtr) = (*srcPtr);
		}
		{
			// bottom left
			GeoPatchContext::VBOVertex *tarPtr = &VBOVtxPtr[(edgeLen - 1) * edgeLen];
			GeoPatchContext::VBOVertex *srcPtr = &VBOVtxPtr[(edgeLen - 2) * edgeLen];
			(*tarPtr) = (*srcPtr);
		}
		{
			// bottom right
			GeoPatchContext::VBOVertex *tarPtr = &VBOVtxPtr[(edgeLen - 1) + ((edgeLen - 1) * edgeLen)];
			GeoPatchContext::VBOVertex *srcPtr = &VBOVtxPtr[(edgeLen - 1) + ((edgeLen - 2) * edgeLen)];
			(*tarPtr) = (*srcPtr);
		}

		// ----------------------------------------------------
		// end of mapping
		vtxBuffer->Unmap();

		// use the new vertex buffer and the shared index buffer
		m_patchVBOData->m_patchMesh.reset(renderer->CreateMeshObject(vtxBuffer, m_ctx->GetIndexBuffer()));

		// Don't need this anymore so throw it away
		//m_patchVBOData->m_heights.reset();
		m_patchVBOData->m_normals.reset();
		m_patchVBOData->m_colors.reset();

#if DEBUG_PATCHES
#ifdef DEBUG_BOUNDING_SPHERES
		RefCountedPtr<Graphics::Material> mat(Pi::renderer->CreateMaterial("unlit", Graphics::MaterialDescriptor(), Graphics::RenderStateDesc()));
		switch (m_depth) {
		case 0: mat->diffuse = Color::WHITE; break;
		case 1: mat->diffuse = Color::RED; break;
		case 2: mat->diffuse = Color::GREEN; break;
		case 3: mat->diffuse = Color::BLUE; break;
		default: mat->diffuse = Color::BLACK; break;
		}
		m_boundsphere.reset(new Graphics::Drawables::Sphere3D(Pi::renderer, mat, 1, m_clipRadius));
#endif
		if (m_label3D == nullptr) {
			std::stringstream stuff;
			stuff << m_PatchID.GetPatchFaceIdx();
			m_label3D.reset(new Graphics::Drawables::Label3D(Pi::renderer, stuff.str()));
		}
#endif // DEBUG_PATCHES
	}
}

// the default sphere we do the horizon culling against
static const SSphere s_sph;
void GeoPatch::Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum)
{
	PROFILE_SCOPED()
	// must update the VBOs to calculate the clipRadius...
	UpdateVBOs(renderer);
	// ...before doing the visibility testing that relies on it.
	if (!IsPatchVisible(frustum, campos))
		return;

#if DEBUG_PATCHES
	RenderLabelDebug(campos, modelView);
#endif // DEBUG_PATCHES

	if (m_kids[0]) {
		for (int i = 0; i < NUM_KIDS; i++)
			m_kids[i]->Render(renderer, campos, modelView, frustum);
	} else if (HasHeightData()) {
		RenderImmediate(renderer, campos, modelView);
	}
}

void GeoPatch::RenderImmediate(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView) const
{
	PROFILE_SCOPED()

#if DEBUG_PATCHES
	RenderLabelDebug(campos, modelView);
#endif // DEBUG_PATCHES

	if (m_patchVBOData->m_heights) {
		const vector3d relpos = m_clipCentroid - campos;
		renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::Translation(relpos)));

		Pi::statSceneTris += (m_ctx->GetNumTris());
		++Pi::statNumPatches;

		// per-patch detail texture scaling value
		const float fDetailFrequency = pow(2.0f, float(m_geosphere->GetMaxDepth()) - float(m_depth));

		m_geosphere->GetSurfaceMaterial()->SetPushConstant("PatchDetailFrequency"_hash, fDetailFrequency);
		renderer->DrawMesh(m_patchVBOData->m_patchMesh.get(), m_geosphere->GetSurfaceMaterial().Get());

#if DEBUG_CENTROIDS
		renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::Translation(m_clipCentroid - campos) * matrix4x4d::ScaleMatrix(0.2 / pow(2.0f, m_depth))));
		Graphics::Drawables::GetAxes3DDrawable(renderer)->Draw(renderer);
#endif

#ifdef DEBUG_BOUNDING_SPHERES
		if (m_boundsphere.get()) {
			renderer->SetWireFrameMode(true);
			m_boundsphere->Draw(renderer);
			renderer->SetWireFrameMode(false);
		}
#endif
		renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_PATCHES, 1);
	}
}

void GeoPatch::GatherRenderablePatches(std::vector<std::pair<double, GeoPatch *>> &visiblePatches, Graphics::Renderer *renderer, const vector3d &campos, const Graphics::Frustum &frustum)
{
	PROFILE_SCOPED()
	// must update the VBOs to calculate the clipRadius...
	UpdateVBOs(renderer);
	// ...before doing the visibility testing that relies on it.
	if (!IsPatchVisible(frustum, campos))
		return;

	if (m_kids[0]) {
		for (int i = 0; i < NUM_KIDS; i++)
			m_kids[i]->GatherRenderablePatches(visiblePatches, renderer, campos, frustum);
	} else if (m_patchVBOData->m_heights) {
		visiblePatches.emplace_back((m_clipCentroid - campos).LengthSqr(), this);
	}
}


void GeoPatch::LODUpdate(const vector3d &campos, const Graphics::Frustum &frustum)
{
	PROFILE_SCOPED()
	// there should be no LOD update when we have active split requests
	if (m_hasJobRequest)
		return;

	bool canSplit = true;
	bool canMerge = bool(m_kids[0]);

	// always split at first level
	double centroidDist = DBL_MAX;
	if (m_depth > 0) {
		centroidDist = (campos - m_clipCentroid).Length();		// distance from camera to centre of the patch
		const bool tooFar = (centroidDist >= m_splitLength);	// check if the distance is greater than the rough length, which is how far it should be before it can split
		if (m_depth >= std::min(GEOPATCH_MAX_DEPTH, m_geosphere->GetMaxDepth()) || tooFar) {
			canSplit = false; // we're too deep in the quadtree or too far away so cannot split
		}
	}

	if (canSplit) {
		if (!m_kids[0]) {
			// Test if this patch is visible
			if (!IsPatchVisible(frustum, campos))
				return;

			// we can see this patch so submit the jobs!
			assert(!m_hasJobRequest);
			m_hasJobRequest = true;

			SQuadSplitRequest *ssrd = new SQuadSplitRequest(m_corners->m_v0, m_corners->m_v1, m_corners->m_v2, m_corners->m_v3, m_clipCentroid.Normalized(), m_depth,
				m_geosphere->GetSystemBody()->GetPath(), m_PatchID, m_ctx->GetEdgeLen() - 2,
				m_ctx->GetFrac(), m_geosphere->GetTerrain());

			// add to the GeoSphere to be processed at end of all LODUpdate requests
			m_geosphere->AddQuadSplitRequest(centroidDist, ssrd, this);
		} else {
			for (int i = 0; i < NUM_KIDS; i++) {
				m_kids[i]->LODUpdate(campos, frustum);
			}
		}
	} else if (canMerge) {
		for (int i = 0; i < NUM_KIDS; i++) {
			canMerge &= m_kids[i]->canBeMerged();
		}
		if (canMerge) {
			for (int i = 0; i < NUM_KIDS; i++) {
				m_kids[i].reset();
			}
		}
	}
}

void GeoPatch::RequestSinglePatch()
{
	if (!HasHeightData()) {
		assert(!m_hasJobRequest);
		m_hasJobRequest = true;
		SSingleSplitRequest *ssrd = new SSingleSplitRequest(m_corners->m_v0, m_corners->m_v1, m_corners->m_v2, m_corners->m_v3, m_clipCentroid.Normalized(), m_depth,
			m_geosphere->GetSystemBody()->GetPath(), m_PatchID, m_ctx->GetEdgeLen() - 2, m_ctx->GetFrac(), m_geosphere->GetTerrain());
		m_job = Pi::GetAsyncJobQueue()->Queue(new SinglePatchJob(ssrd));
	}
}

void GeoPatch::ReceiveHeightmaps(SQuadSplitResult *psr)
{
	PROFILE_SCOPED()
	assert(NULL != psr);
	if (m_depth < psr->depth()) {
		// this should work because each depth should have a common history
		const Uint32 kidIdx = psr->data(0).patchID.GetPatchIdx(m_depth + 1);
		if (m_kids[kidIdx]) {
			m_kids[kidIdx]->ReceiveHeightmaps(psr);
		} else {
			psr->OnCancel();
		}
	} else {
		assert(m_hasJobRequest);
		const int newDepth = m_depth + 1;
		for (int i = 0; i < NUM_KIDS; i++) {
			assert(!m_kids[i]);
			const SSplitResultData &data = psr->data(i);
			assert(i == data.patchID.GetPatchIdx(newDepth));
			assert(0 == data.patchID.GetPatchIdx(newDepth + 1));
			m_kids[i].reset(new GeoPatch(m_ctx, m_geosphere,
				data.v0, data.v1, data.v2, data.v3,
				newDepth, data.patchID));
		}

		for (int i = 0; i < NUM_KIDS; i++) {
			m_kids[i]->ReceiveHeightResult(psr->data(i));
		}
		m_hasJobRequest = false;
	}
}

void GeoPatch::ReceiveHeightmap(const SSingleSplitResult *psr)
{
	PROFILE_SCOPED()
	assert(nullptr != psr);
	assert(m_hasJobRequest);

	ReceiveHeightResult(psr->data());
	m_hasJobRequest = false;
}

void GeoPatch::ReceiveHeightResult(const SSplitResultData &data)
{
	m_patchVBOData.reset(new PatchVBOData(data.heights, data.normals, data.colors));

	// skirt vertices are not present in the heights array
	const int edgeLen = m_ctx->GetEdgeLen() - 2;

	const double h0 = data.heights[0];
	const double h1 = data.heights[edgeLen - 1];
	const double h2 = data.heights[edgeLen * (edgeLen - 1)];
	const double h3 = data.heights[edgeLen * edgeLen - 1];

	const double height = (h0 + h1 + h2 + h3) * 0.25;
	m_clipCentroid *= (1.0 + height);

	SetNeedToUpdateVBOs();
}

void GeoPatch::ReceiveJobHandle(Job::Handle job)
{
	assert(!m_job.HasJob());
	m_job = static_cast<Job::Handle &&>(job);
}

bool GeoPatch::IsPatchVisible(const Graphics::Frustum &frustum, const vector3d &camPos) const
{
	PROFILE_SCOPED()
	// Test if this patch is visible
	if (!frustum.TestSphere(m_clipCentroid, m_clipRadius))
		return false; // nothing below this patch is visible

	// We only want to horizon cull patches that can actually be over the horizon!
	// depth == zero patches cannot be tested against the horizon
	if (m_depth != 0 && IsOverHorizon(camPos)) {
		return false;
	}

	return true;
}

bool GeoPatch::IsOverHorizon(const vector3d &camPos) const
{
	PROFILE_SCOPED()
	const vector3d camDir(camPos - m_clipCentroid);
	const vector3d camDirNorm(camDir.Normalized());
	const vector3d cenDir(m_clipCentroid.Normalized());
	const double dotProd = camDirNorm.Dot(cenDir);

	if (dotProd < 0.25 && (camDir.LengthSqr() > (m_clipRadius * m_clipRadius))) {
		// return the result of the Horizon Culling test, inverted to match naming semantic
		// eg: HC returns true==visible, but this method returns true==hidden
#if USE_SUB_CENTROID_CLIPPING
		bool gather = false;
		for (size_t i = 0; i < NUM_HORIZON_POINTS; i++) {
			gather |= s_sph.HorizonCulling(camPos, m_clipHorizon[i]);
			if (gather) 
				return false; // early out if any test is visible
		}
		return !gather; // if true then it's visible, return semantic is true == over the horizon and thus invisible so invert
#else
		return !s_sph.HorizonCulling(camPos, SSphere(m_clipCentroid, m_clipRadius));
#endif // #if USE_SUB_CENTROID_CLIPPING
	}

	// not over the horizon
	return false;
}

#if DEBUG_PATCHES
void GeoPatch::RenderLabelDebug(const vector3d &campos, const matrix4x4d &modelView) const
{
	if (m_label3D != nullptr) {
		const vector3d relpos = m_clipCentroid - campos;
		m_label3D->Draw(Pi::renderer, matrix4x4f(modelView * matrix4x4d::Translation(relpos)));
	}
}
#endif // DEBUG_PATCHES
