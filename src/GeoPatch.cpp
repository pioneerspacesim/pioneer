// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoPatch.h"

#include "GeoPatchContext.h"
#include "GeoPatchJobs.h"
#include "GeoSphere.h"
#include "MathUtil.h"
#include "Pi.h"
#include "RefCounted.h"
#include "Sphere.h"
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

#define DEBUG_CENTROIDS 0

#ifdef DEBUG_BOUNDING_SPHERES
#include "graphics/RenderState.h"
#endif

#if DEBUG_CENTROIDS
#include "graphics/Drawables.h"
#endif

// tri edge lengths
static const double GEOPATCH_SUBDIVIDE_AT_CAMDIST = 5.0;

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs,
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
	const int depth, const GeoPatchID &ID_) :
	m_ctx(ctx_),
	m_v0(v0_),
	m_v1(v1_),
	m_v2(v2_),
	m_v3(v3_),
	m_heights(nullptr),
	m_normals(nullptr),
	m_colors(nullptr),
	m_parent(nullptr),
	m_geosphere(gs),
	m_depth(depth),
	m_PatchID(ID_),
	m_HasJobRequest(false)
{

	m_clipCentroid = (m_v0 + m_v1 + m_v2 + m_v3) * 0.25;
	m_centroid = m_clipCentroid.Normalized();
	m_clipRadius = 0.0;
	m_clipRadius = std::max(m_clipRadius, (m_v0 - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (m_v1 - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (m_v2 - m_clipCentroid).Length());
	m_clipRadius = std::max(m_clipRadius, (m_v3 - m_clipCentroid).Length());
	double distMult;
	if (m_geosphere->GetSystemBody()->GetType() < SystemBody::TYPE_PLANET_ASTEROID) {
		distMult = 10.0 / Clamp(m_depth, 1, 10);
	} else {
		distMult = 5.0 / Clamp(m_depth, 1, 5);
	}
	m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, m_depth) * distMult;
	m_needUpdateVBOs = false;
}

GeoPatch::~GeoPatch()
{
	m_HasJobRequest = false;
	for (int i = 0; i < NUM_KIDS; i++) {
		m_kids[i].reset();
	}
	m_heights.reset();
	m_normals.reset();
	m_colors.reset();
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
		const double *pHts = m_heights.get();
		const vector3f *pNorm = m_normals.get();
		const Color3ub *pColr = m_colors.get();

		double minh = DBL_MAX;

		// ----------------------------------------------------
		// inner loops
		for (Sint32 y = 1; y < edgeLen - 1; y++) {
			for (Sint32 x = 1; x < edgeLen - 1; x++) {
				const double height = *pHts;
				minh = std::min(height, minh);
				const double xFrac = double(x - 1) * frac;
				const double yFrac = double(y - 1) * frac;
				const vector3d p((GetSpherePoint(xFrac, yFrac) * (height + 1.0)) - m_clipCentroid);
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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - m_clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - m_clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - m_clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - m_clipCentroid);

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
		m_patchMesh.reset(renderer->CreateMeshObject(vtxBuffer, m_ctx->GetIndexBuffer()));

		// Don't need this anymore so throw it away
		m_normals.reset();
		m_colors.reset();

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
	}
}

// the default sphere we do the horizon culling against
static const SSphere s_sph;
void GeoPatch::Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum)
{
	PROFILE_SCOPED()
	// must update the VBOs to calculate the clipRadius...
	UpdateVBOs(renderer);
	// ...before doing the furstum culling that relies on it.
	if (!frustum.TestPoint(m_clipCentroid, m_clipRadius))
		return; // nothing below this patch is visible

	// only want to horizon cull patches that can actually be over the horizon!
	const vector3d camDir(campos - m_clipCentroid);
	const vector3d camDirNorm(camDir.Normalized());
	const vector3d cenDir(m_clipCentroid.Normalized());
	const double dotProd = camDirNorm.Dot(cenDir);

	if (dotProd < 0.25 && (camDir.LengthSqr() > (m_clipRadius * m_clipRadius))) {
		SSphere obj;
		obj.m_centre = m_clipCentroid;
		obj.m_radius = m_clipRadius;

		if (!s_sph.HorizonCulling(campos, obj)) {
			return; // nothing below this patch is visible
		}
	}

	if (m_kids[0]) {
		for (int i = 0; i < NUM_KIDS; i++)
			m_kids[i]->Render(renderer, campos, modelView, frustum);
	} else if (m_heights) {
		const vector3d relpos = m_clipCentroid - campos;
		renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::Translation(relpos)));

		Pi::statSceneTris += (m_ctx->GetNumTris());
		++Pi::statNumPatches;

		// per-patch detail texture scaling value
		const float fDetailFrequency = pow(2.0f, float(m_geosphere->GetMaxDepth()) - float(m_depth));

		m_geosphere->GetSurfaceMaterial()->SetPushConstant("PatchDetailFrequency"_hash, fDetailFrequency);
		renderer->DrawMesh(m_patchMesh.get(), m_geosphere->GetSurfaceMaterial().Get());

#if DEBUG_CENTROIDS
		renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::Translation(m_centroid - campos) * matrix4x4d::ScaleMatrix(0.2 / pow(2.0f, m_depth))));
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

void GeoPatch::LODUpdate(const vector3d &campos, const Graphics::Frustum &frustum)
{
	// there should be no LOD update when we have active split requests
	if (m_HasJobRequest)
		return;

	bool canSplit = true;
	bool canMerge = bool(m_kids[0]);

	// always split at first level
	double centroidDist = DBL_MAX;
	if (m_parent) {
		centroidDist = (campos - m_centroid).Length();		 // distance from camera to centre of the patch
		const bool tooFar = (centroidDist >= m_roughLength); // check if the distance is greater than the rough length, which is how far it should be before it can split
		if (m_depth >= std::min(GEOPATCH_MAX_DEPTH, m_geosphere->GetMaxDepth()) || tooFar) {
			canSplit = false; // we're too deep in the quadtree or too far away so cannot split
		}
	}

	if (canSplit) {
		if (!m_kids[0]) {
			// Test if this patch is visible
			if (!frustum.TestPoint(m_clipCentroid, m_clipRadius))
				return; // nothing below this patch is visible

			// only want to horizon cull patches that can actually be over the horizon!
			const vector3d camDir(campos - m_clipCentroid);
			const vector3d camDirNorm(camDir.Normalized());
			const vector3d cenDir(m_clipCentroid.Normalized());
			const double dotProd = camDirNorm.Dot(cenDir);

			if (dotProd < 0.25 && (camDir.LengthSqr() > (m_clipRadius * m_clipRadius))) {
				SSphere obj;
				obj.m_centre = m_clipCentroid;
				obj.m_radius = m_clipRadius;

				if (!s_sph.HorizonCulling(campos, obj)) {
					return; // nothing below this patch is visible
				}
			}

			// we can see this patch so submit the jobs!
			assert(!m_HasJobRequest);
			m_HasJobRequest = true;

			SQuadSplitRequest *ssrd = new SQuadSplitRequest(m_v0, m_v1, m_v2, m_v3, m_centroid.Normalized(), m_depth,
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
	if (!m_heights) {
		assert(!m_HasJobRequest);
		m_HasJobRequest = true;
		SSingleSplitRequest *ssrd = new SSingleSplitRequest(m_v0, m_v1, m_v2, m_v3, m_centroid.Normalized(), m_depth,
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
		assert(m_HasJobRequest);
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
		m_kids[0]->m_parent = m_kids[1]->m_parent = m_kids[2]->m_parent = m_kids[3]->m_parent = this;

		for (int i = 0; i < NUM_KIDS; i++) {
			m_kids[i]->ReceiveHeightResult(psr->data(i));
		}
		m_HasJobRequest = false;
	}
}

void GeoPatch::ReceiveHeightmap(const SSingleSplitResult *psr)
{
	PROFILE_SCOPED()
	assert(nullptr == m_parent);
	assert(nullptr != psr);
	assert(m_HasJobRequest);

	ReceiveHeightResult(psr->data());
	m_HasJobRequest = false;
}

void GeoPatch::ReceiveHeightResult(const SSplitResultData &data)
{
	m_heights.reset(data.heights);
	m_normals.reset(data.normals);
	m_colors.reset(data.colors);

	// skirt vertices are not present in the heights array
	const int edgeLen = m_ctx->GetEdgeLen() - 2;

	const double h0 = m_heights[0];
	const double h1 = m_heights[edgeLen - 1];
	const double h2 = m_heights[edgeLen * (edgeLen - 1)];
	const double h3 = m_heights[edgeLen * edgeLen - 1];

	const double height = (h0 + h1 + h2 + h3) * 0.25;
	m_centroid *= (1.0 + height);

	NeedToUpdateVBOs();
}

void GeoPatch::ReceiveJobHandle(Job::Handle job)
{
	assert(!m_job.HasJob());
	m_job = static_cast<Job::Handle &&>(job);
}
