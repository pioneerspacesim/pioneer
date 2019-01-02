// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoPatch.h"
#include "GeoPatchContext.h"
#include "GeoPatchJobs.h"
#include "GeoSphere.h"
#include "MathUtil.h"
#include "Pi.h"
#include "RefCounted.h"
#include "Sphere.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "libs.h"
#include "perlin.h"
#include "vcacheopt/vcacheopt.h"
#include <algorithm>
#include <deque>

// tri edge lengths
static const double GEOPATCH_SUBDIVIDE_AT_CAMDIST = 5.0;

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs,
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
	const int depth, const GeoPatchID &ID_) :
	ctx(ctx_),
	v0(v0_),
	v1(v1_),
	v2(v2_),
	v3(v3_),
	heights(nullptr),
	normals(nullptr),
	colors(nullptr),
	parent(nullptr),
	geosphere(gs),
	m_depth(depth),
	mPatchID(ID_),
	mHasJobRequest(false)
{

	clipCentroid = (v0 + v1 + v2 + v3) * 0.25;
	centroid = clipCentroid.Normalized();
	clipRadius = 0.0;
	clipRadius = std::max(clipRadius, (v0 - clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v1 - clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v2 - clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v3 - clipCentroid).Length());
	double distMult;
	if (geosphere->GetSystemBody()->GetType() < SystemBody::TYPE_PLANET_ASTEROID) {
		distMult = 10.0 / Clamp(depth, 1, 10);
	} else {
		distMult = 5.0 / Clamp(depth, 1, 5);
	}
	m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth) * distMult;
	m_needUpdateVBOs = false;
}

GeoPatch::~GeoPatch()
{
	mHasJobRequest = false;
	for (int i = 0; i < NUM_KIDS; i++) {
		kids[i].reset();
	}
	heights.reset();
	normals.reset();
	colors.reset();
}

void GeoPatch::UpdateVBOs(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	if (m_needUpdateVBOs) {
		assert(renderer);
		m_needUpdateVBOs = false;

		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
		vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[2].semantic = Graphics::ATTRIB_DIFFUSE;
		vbd.attrib[2].format = Graphics::ATTRIB_FORMAT_UBYTE4;
		vbd.attrib[3].semantic = Graphics::ATTRIB_UV0;
		vbd.attrib[3].format = Graphics::ATTRIB_FORMAT_FLOAT2;
		vbd.numVertices = ctx->NUMVERTICES();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		m_vertexBuffer.reset(renderer->CreateVertexBuffer(vbd));

		GeoPatchContext::VBOVertex *VBOVtxPtr = m_vertexBuffer->Map<GeoPatchContext::VBOVertex>(Graphics::BUFFER_MAP_WRITE);
		assert(m_vertexBuffer->GetDesc().stride == sizeof(GeoPatchContext::VBOVertex));

		const Sint32 edgeLen = ctx->GetEdgeLen();
		const double frac = ctx->GetFrac();
		const double *pHts = heights.get();
		const vector3f *pNorm = normals.get();
		const Color3ub *pColr = colors.get();

		double minh = DBL_MAX;

		// ----------------------------------------------------
		// inner loops
		for (Sint32 y = 1; y < edgeLen - 1; y++) {
			for (Sint32 x = 1; x < edgeLen - 1; x++) {
				const double height = *pHts;
				minh = std::min(height, minh);
				const double xFrac = double(x - 1) * frac;
				const double yFrac = double(y - 1) * frac;
				const vector3d p((GetSpherePoint(xFrac, yFrac) * (height + 1.0)) - clipCentroid);
				clipRadius = std::max(clipRadius, p.Length());

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - clipCentroid);

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
			const vector3d p((GetSpherePoint(xFrac, yFrac) * minhScale) - clipCentroid);

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
		m_vertexBuffer->Unmap();

		// Don't need this anymore so throw it away
		normals.reset();
		colors.reset();

#ifdef DEBUG_BOUNDING_SPHERES
		RefCountedPtr<Graphics::Material> mat(Pi::renderer->CreateMaterial(Graphics::MaterialDescriptor()));
		m_boundsphere.reset(new Graphics::Drawables::Sphere3D(Pi::renderer, mat, Pi::renderer->CreateRenderState(Graphics::RenderStateDesc()), 0, clipRadius));
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
	if (!frustum.TestPoint(clipCentroid, clipRadius))
		return; // nothing below this patch is visible

	// only want to horizon cull patches that can actually be over the horizon!
	const vector3d camDir(campos - clipCentroid);
	const vector3d camDirNorm(camDir.Normalized());
	const vector3d cenDir(clipCentroid.Normalized());
	const double dotProd = camDirNorm.Dot(cenDir);

	if (dotProd < 0.25 && (camDir.LengthSqr() > (clipRadius * clipRadius))) {
		SSphere obj;
		obj.m_centre = clipCentroid;
		obj.m_radius = clipRadius;

		if (!s_sph.HorizonCulling(campos, obj)) {
			return; // nothing below this patch is visible
		}
	}

	if (kids[0]) {
		for (int i = 0; i < NUM_KIDS; i++)
			kids[i]->Render(renderer, campos, modelView, frustum);
	} else if (heights) {
		RefCountedPtr<Graphics::Material> mat = geosphere->GetSurfaceMaterial();
		Graphics::RenderState *rs = geosphere->GetSurfRenderState();

		const vector3d relpos = clipCentroid - campos;
		renderer->SetTransform(modelView * matrix4x4d::Translation(relpos));

		Pi::statSceneTris += (ctx->GetNumTris());
		++Pi::statNumPatches;

		// per-patch detail texture scaling value
		geosphere->GetMaterialParameters().patchDepth = m_depth;

		renderer->DrawBufferIndexed(m_vertexBuffer.get(), ctx->GetIndexBuffer(), rs, mat.Get());
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
	if (mHasJobRequest)
		return;

	bool canSplit = true;
	bool canMerge = bool(kids[0]);

	// always split at first level
	double centroidDist = DBL_MAX;
	if (parent) {
		centroidDist = (campos - centroid).Length();
		const bool errorSplit = (centroidDist < m_roughLength);
		if (!(canSplit && (m_depth < std::min(GEOPATCH_MAX_DEPTH, geosphere->GetMaxDepth())) && errorSplit)) {
			canSplit = false;
		}
	}

	if (canSplit) {
		if (!kids[0]) {
			// Test if this patch is visible
			if (!frustum.TestPoint(clipCentroid, clipRadius))
				return; // nothing below this patch is visible

			// only want to horizon cull patches that can actually be over the horizon!
			const vector3d camDir(campos - clipCentroid);
			const vector3d camDirNorm(camDir.Normalized());
			const vector3d cenDir(clipCentroid.Normalized());
			const double dotProd = camDirNorm.Dot(cenDir);

			if (dotProd < 0.25 && (camDir.LengthSqr() > (clipRadius * clipRadius))) {
				SSphere obj;
				obj.m_centre = clipCentroid;
				obj.m_radius = clipRadius;

				if (!s_sph.HorizonCulling(campos, obj)) {
					return; // nothing below this patch is visible
				}
			}

			// we can see this patch so submit the jobs!
			assert(!mHasJobRequest);
			mHasJobRequest = true;

			SQuadSplitRequest *ssrd = new SQuadSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
				geosphere->GetSystemBody()->GetPath(), mPatchID, ctx->GetEdgeLen() - 2,
				ctx->GetFrac(), geosphere->GetTerrain());

			// add to the GeoSphere to be processed at end of all LODUpdate requests
			geosphere->AddQuadSplitRequest(centroidDist, ssrd, this);
		} else {
			for (int i = 0; i < NUM_KIDS; i++) {
				kids[i]->LODUpdate(campos, frustum);
			}
		}
	} else if (canMerge) {
		for (int i = 0; i < NUM_KIDS; i++) {
			canMerge &= kids[i]->canBeMerged();
		}
		if (canMerge) {
			for (int i = 0; i < NUM_KIDS; i++) {
				kids[i].reset();
			}
		}
	}
}

void GeoPatch::RequestSinglePatch()
{
	if (!heights) {
		assert(!mHasJobRequest);
		mHasJobRequest = true;
		SSingleSplitRequest *ssrd = new SSingleSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
			geosphere->GetSystemBody()->GetPath(), mPatchID, ctx->GetEdgeLen() - 2, ctx->GetFrac(), geosphere->GetTerrain());
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
		if (kids[kidIdx]) {
			kids[kidIdx]->ReceiveHeightmaps(psr);
		} else {
			psr->OnCancel();
		}
	} else {
		assert(mHasJobRequest);
		const int nD = m_depth + 1;
		for (int i = 0; i < NUM_KIDS; i++) {
			assert(!kids[i]);
			const SQuadSplitResult::SSplitResultData &data = psr->data(i);
			assert(i == data.patchID.GetPatchIdx(nD));
			assert(0 == data.patchID.GetPatchIdx(nD + 1));
			kids[i].reset(new GeoPatch(ctx, geosphere,
				data.v0, data.v1, data.v2, data.v3,
				nD, data.patchID));
		}
		kids[0]->parent = kids[1]->parent = kids[2]->parent = kids[3]->parent = this;

		for (int i = 0; i < NUM_KIDS; i++) {
			const SQuadSplitResult::SSplitResultData &data = psr->data(i);
			kids[i]->heights.reset(data.heights);
			kids[i]->normals.reset(data.normals);
			kids[i]->colors.reset(data.colors);
		}
		for (int i = 0; i < NUM_KIDS; i++) {
			kids[i]->NeedToUpdateVBOs();
		}
		mHasJobRequest = false;
	}
}

void GeoPatch::ReceiveHeightmap(const SSingleSplitResult *psr)
{
	PROFILE_SCOPED()
	assert(nullptr == parent);
	assert(nullptr != psr);
	assert(mHasJobRequest);
	{
		const SSingleSplitResult::SSplitResultData &data = psr->data();
		heights.reset(data.heights);
		normals.reset(data.normals);
		colors.reset(data.colors);
	}
	mHasJobRequest = false;
}

void GeoPatch::ReceiveJobHandle(Job::Handle job)
{
	assert(!m_job.HasJob());
	m_job = static_cast<Job::Handle &&>(job);
}
