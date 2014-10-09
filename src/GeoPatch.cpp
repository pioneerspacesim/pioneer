// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoPatchContext.h"
#include "GeoPatch.h"
#include "GeoPatchJobs.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"
#include "MathUtil.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>

struct SPlane {
	double a, b, c, d;
	double DistanceToPoint(const vector3d &p) const {
		return a*p.x + b*p.y + c*p.z + d;
	}

	SPlane::SPlane(const vector3d& N, const vector3d &P)
	{
		const vector3d NormalizedNormal = N.Normalized();
		a = NormalizedNormal.x;
		b = NormalizedNormal.y;
		c = NormalizedNormal.z;
		d = -(P.Dot(NormalizedNormal));
	}
};


struct SSphere3D {
	SSphere3D() : m_centre(vector3d(0.0)), m_radius(1.0) {}
	vector3d m_centre;
	double m_radius;

	// Adapted from Ysaneya here: http://www.gamedev.net/blog/73/entry-1666972-horizon-culling/
	///
	///		Performs horizon culling with an object's bounding sphere, given a view point.
	///		This function checks whether the object's sphere is inside the view cone formed
	///		by the view point and this sphere. The view cone is capped so that the visibility
	///		is false only in the 'shadow' of this sphere.
	///		@param view Position of view point in world space
	///		@param obj Bounding sphere of another object.
	///		@return true if the object's bounding sphere is visible from the viewpoint, false if the
	///		sphere is in the shadow cone AND not in front of the capping plane.
	///
	bool SSphere3D::HorizonCulling(const vector3d& view, const SSphere3D& obj) const
	{
		vector3d O1C = m_centre - view;
		vector3d O2C = obj.m_centre - view;

		const double D1 = O1C.Length();
		const double D2 = O2C.Length();
		const double R1 = m_radius;
		const double R2 = obj.m_radius;
		const double iD1 = 1.0f / D1;
		const double iD2 = 1.0f / D2;
	
		O1C *= iD1;
		O2C *= iD2;
		const double K = O1C.Dot(O2C);

		const double K1 = R1 * iD1;
		const double K2 = R2 * iD2;
		bool status = true;
		if ( K > K1 * K2 )
		{
			status = (-2.0f * K * K1 * K2 + K1 * K1 + K2 * K2 < 1.0f - K * K);
		}

		const double y = R1 * R1 * iD1;
		const vector3d P = m_centre - y * O1C;
		const vector3d N = -O1C;
		SPlane plane(N, P);
		status = status || (plane.DistanceToPoint(obj.m_centre) > obj.m_radius);

		return status;
	}
};

// tri edge lengths
static const double GEOPATCH_SUBDIVIDE_AT_CAMDIST = 5.0;
#define GEOPATCH_MAX_DEPTH  15 + (2*Pi::detail.fracmult) //15

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs,
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
	const int depth, const GeoPatchID &ID_)
	: ctx(ctx_), v0(v0_), v1(v1_), v2(v2_), v3(v3_),
	heights(nullptr), normals(nullptr), colors(nullptr),
	parent(nullptr), geosphere(gs),
	m_depth(depth), mPatchID(ID_),
	mHasJobRequest(false)
{
	for (int i=0; i<NUM_KIDS; ++i) {
		edgeFriend[i]	= NULL;
	}

	clipCentroid = (v0+v1+v2+v3) * 0.25;
	centroid = clipCentroid.Normalized();
	clipRadius = 0.0;
	clipRadius = std::max(clipRadius, (v0-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v1-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v2-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v3-clipCentroid).Length());
	double distMult;
	if (geosphere->GetSystemBody()->GetType() < SystemBody::TYPE_PLANET_ASTEROID) {
 		distMult = 10.0 / Clamp(depth, 1, 10);
 	} else {
 		distMult = 5.0 / Clamp(depth, 1, 5);
 	}
	m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth) * distMult;
	m_needUpdateVBOs = false;
}

GeoPatch::~GeoPatch() {
	mHasJobRequest = false;

	for (int i=0; i<NUM_KIDS; i++) {
		if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
	}
	for (int i=0; i<NUM_KIDS; i++) {
		kids[i].reset();
	}
	heights.reset();
	normals.reset();
	colors.reset();
}

void GeoPatch::_UpdateVBOs(Graphics::Renderer *renderer) 
{
	if (m_needUpdateVBOs) {
		assert(renderer);
		m_needUpdateVBOs = false;

		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
		vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[2].semantic = Graphics::ATTRIB_DIFFUSE;
		vbd.attrib[2].format   = Graphics::ATTRIB_FORMAT_UBYTE4;
		vbd.numVertices = ctx->NUMVERTICES();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		m_vertexBuffer.reset(renderer->CreateVertexBuffer(vbd));

		GeoPatchContext::VBOVertex* vtxPtr = m_vertexBuffer->Map<GeoPatchContext::VBOVertex>(Graphics::BUFFER_MAP_WRITE);
		assert(m_vertexBuffer->GetDesc().stride == sizeof(GeoPatchContext::VBOVertex));

		const Sint32 edgeLen = ctx->GetEdgeLen();
		const double frac = ctx->GetFrac();
		const double *pHts = heights.get();
		const vector3f *pNorm = normals.get();
		const Color3ub *pColr = colors.get();
		for (Sint32 y=0; y<edgeLen; y++) {
			for (Sint32 x=0; x<edgeLen; x++) {
				const double height = *pHts;
				const double xFrac = double(x)*frac;
				const double yFrac = double(y)*frac;
				const vector3d p((GetSpherePoint(xFrac, yFrac) * (height + 1.0)) - clipCentroid);
				clipRadius = std::max(clipRadius, p.Length());
				vtxPtr->pos = vector3f(p);
				++pHts;	// next height

				const vector3f norma(pNorm->Normalized());
				vtxPtr->norm = norma;
				++pNorm; // next normal

				vtxPtr->col[0] = pColr->r;
				vtxPtr->col[1] = pColr->g;
				vtxPtr->col[2] = pColr->b;
				vtxPtr->col[3] = 255;
				++pColr; // next colour

				++vtxPtr; // next vertex
			}
		}
		m_vertexBuffer->Unmap();
	}
}


static const SSphere3D s_sph;
static bool s_buseHC = true;
#pragma optimize("",off)
void GeoPatch::Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum)
{
	if(s_buseHC)
	{
		SSphere3D obj;
		obj.m_centre = clipCentroid;
		obj.m_radius = clipRadius;

		if( !s_sph.HorizonCulling(campos, obj) )
			return;
	}

	if (kids[0]) {
		for (int i=0; i<NUM_KIDS; i++) kids[i]->Render(renderer, campos, modelView, frustum);
	} else if (heights) {
		_UpdateVBOs(renderer);

		if (!frustum.TestPoint(clipCentroid, clipRadius))
			return;

		Graphics::Material *mat = geosphere->GetSurfaceMaterial();
		Graphics::RenderState *rs = geosphere->GetSurfRenderState();

		const vector3d relpos = clipCentroid - campos;
		renderer->SetTransform(modelView * matrix4x4d::Translation(relpos));

		Pi::statSceneTris += (ctx->GetNumTris());
		++Pi::statNumPatches;

		renderer->DrawBufferIndexed(m_vertexBuffer.get(), ctx->GetIndexBuffer(DetermineIndexbuffer()), rs, mat);
	}
}
#pragma optimize("",off)
void GeoPatch::LODUpdate(const vector3d &campos) 
{
	// there should be no LOD update when we have active split requests
	if(mHasJobRequest)
		return;

	/*SSphere3D obj;
	obj.m_centre = clipCentroid;
	obj.m_radius = clipRadius;

	if( !s_sph.HorizonCulling(campos, obj) )
		return;*/

	bool canSplit = true;
	bool canMerge = bool(kids[0]);

	// always split at first level
	if (parent) {
		for (int i=0; i<NUM_EDGES; i++) {
			if (!edgeFriend[i]) {
				canSplit = false;
				break;
			} else if (edgeFriend[i]->m_depth < m_depth) {
				canSplit = false;
				break;
			}
		}
		const float centroidDist = (campos - centroid).Length();
		const bool errorSplit = (centroidDist < m_roughLength);
		if( !(canSplit && (m_depth < std::min(GEOPATCH_MAX_DEPTH, geosphere->GetMaxDepth())) && errorSplit) ) {
			canSplit = false;
		}
	}

	if (canSplit) {
		if (!kids[0]) {
            assert(!mHasJobRequest);
            assert(!m_job.HasJob());
			mHasJobRequest = true;

			SQuadSplitRequest *ssrd = new SQuadSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
						geosphere->GetSystemBody()->GetPath(), mPatchID, ctx->GetEdgeLen(),
						ctx->GetFrac(), geosphere->GetTerrain());
			m_job = Pi::GetAsyncJobQueue()->Queue(new QuadPatchJob(ssrd));
		} else {
			for (int i=0; i<NUM_KIDS; i++) {
				kids[i]->LODUpdate(campos);
			}
		}
	} else if (canMerge) {
		for (int i=0; i<NUM_KIDS; i++) {
			canMerge &= kids[i]->canBeMerged();
		}
		if( canMerge ) {
			for (int i=0; i<NUM_KIDS; i++) {
				kids[i].reset();
			}
		}
	}
}

void GeoPatch::RequestSinglePatch()
{
	if( !heights ) {
        assert(!mHasJobRequest);
        assert(!m_job.HasJob());
		mHasJobRequest = true;
		SSingleSplitRequest *ssrd = new SSingleSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
					geosphere->GetSystemBody()->GetPath(), mPatchID, ctx->GetEdgeLen(), ctx->GetFrac(), geosphere->GetTerrain());
		m_job = Pi::GetAsyncJobQueue()->Queue(new SinglePatchJob(ssrd));
	}
}

void GeoPatch::ReceiveHeightmaps(SQuadSplitResult *psr)
{
	assert(NULL!=psr);
	if (m_depth<psr->depth()) {
		// this should work because each depth should have a common history
		const Uint32 kidIdx = psr->data(0).patchID.GetPatchIdx(m_depth+1);
		if( kids[kidIdx] ) {
			kids[kidIdx]->ReceiveHeightmaps(psr);
		} else {
			psr->OnCancel();
		}
	} else {
		assert(mHasJobRequest);
		const int nD = m_depth+1;
		for (int i=0; i<NUM_KIDS; i++)
		{
			assert(!kids[i]);
			const SQuadSplitResult::SSplitResultData& data = psr->data(i);
			assert(i==data.patchID.GetPatchIdx(nD));
			assert(0==data.patchID.GetPatchIdx(nD+1));
			kids[i].reset(new GeoPatch(ctx, geosphere,
				data.v0, data.v1, data.v2, data.v3,
				nD, data.patchID));
		}

		// hm.. edges. Not right to pass this
		// edgeFriend...
		kids[0]->edgeFriend[0] = GetEdgeFriendForKid(0, 0);
		kids[0]->edgeFriend[1] = kids[1].get();
		kids[0]->edgeFriend[2] = kids[3].get();
		kids[0]->edgeFriend[3] = GetEdgeFriendForKid(0, 3);
		kids[1]->edgeFriend[0] = GetEdgeFriendForKid(1, 0);
		kids[1]->edgeFriend[1] = GetEdgeFriendForKid(1, 1);
		kids[1]->edgeFriend[2] = kids[2].get();
		kids[1]->edgeFriend[3] = kids[0].get();
		kids[2]->edgeFriend[0] = kids[1].get();
		kids[2]->edgeFriend[1] = GetEdgeFriendForKid(2, 1);
		kids[2]->edgeFriend[2] = GetEdgeFriendForKid(2, 2);
		kids[2]->edgeFriend[3] = kids[3].get();
		kids[3]->edgeFriend[0] = kids[0].get();
		kids[3]->edgeFriend[1] = kids[2].get();
		kids[3]->edgeFriend[2] = GetEdgeFriendForKid(3, 2);
		kids[3]->edgeFriend[3] = GetEdgeFriendForKid(3, 3);
		kids[0]->parent = kids[1]->parent = kids[2]->parent = kids[3]->parent = this;

		for (int i=0; i<NUM_KIDS; i++)
		{
			const SQuadSplitResult::SSplitResultData& data = psr->data(i);
			kids[i]->heights.reset(data.heights);
			kids[i]->normals.reset(data.normals);
			kids[i]->colors.reset(data.colors);
		}
		for (int i=0; i<NUM_EDGES; i++) { if(edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendSplit(this); }
		for (int i=0; i<NUM_KIDS; i++) {
			kids[i]->UpdateVBOs();
		}
		mHasJobRequest = false;
	}
}

void GeoPatch::ReceiveHeightmap(const SSingleSplitResult *psr)
{
	assert(nullptr == parent);
	assert(nullptr != psr);
	assert(mHasJobRequest);
	{
		const SSingleSplitResult::SSplitResultData& data = psr->data();
		heights.reset(data.heights);
		normals.reset(data.normals);
		colors.reset(data.colors);
	}
	mHasJobRequest = false;
}
