// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCH_H
#define _GEOPATCH_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"
#include "jobswarm/JobManager.h"

#include <deque>

namespace Graphics { class Renderer; class Frustum; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere;

class SSplitRequestDescription {
public:
	SSplitRequestDescription(const vector3d &v0_,
							const vector3d &v1_,
							const vector3d &v2_,
							const vector3d &v3_,
							const vector3d &cn,
							const uint32_t depth_,
							const SystemPath &sysPath_,
							const GeoPatchID &patchID_,
							const int edgeLen_,
							const double fracStep_,
							Terrain *pTerrain_,
							GeoSphere *pGeoSphere_)
							: v0(v0_), v1(v1_), v2(v2_), v3(v3_), centroid(cn), depth(depth_), 
							sysPath(sysPath_), patchID(patchID_), edgeLen(edgeLen_), fracStep(fracStep_), 
							pTerrain(pTerrain_), 
							pGeoSphere(pGeoSphere_)
	{
		const int numVerts = NUMVERTICES(edgeLen_);
		for( int i=0 ; i<4 ; ++i )
		{
			vertices[i] = new vector3d[numVerts];
			normals[i] = new vector3d[numVerts];
			colors[i] = new vector3d[numVerts];
		}
	}

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }
	inline int NUMVERTICES(const int el) const { return el*el; }

	const vector3d v0;
	const vector3d v1;
	const vector3d v2;
	const vector3d v3;
	const vector3d centroid;
	const uint32_t depth;
	const SystemPath sysPath;
	const GeoPatchID patchID;
	const int edgeLen;
	const double fracStep;
	Terrain *pTerrain;
	// quick hack, do not have in the final version!
	GeoSphere *pGeoSphere;
	vector3d *vertices[4];
	vector3d *normals[4];
	vector3d *colors[4];

private:
	// deliberately prevent copy constructor access
	SSplitRequestDescription(const SSplitRequestDescription &r) : v0(r.v0), v1(r.v1), v2(r.v2), v3(r.v3), centroid(r.centroid), depth(r.depth), 
							sysPath(r.sysPath), patchID(r.patchID), edgeLen(r.edgeLen), fracStep(r.fracStep), 
							pTerrain(r.pTerrain), 
							pGeoSphere(r.pGeoSphere)
	{
		assert(false);
		for( int i=0 ; i<4 ; ++i )
		{
			vertices[i] = r.vertices[i];
			normals[i] = r.normals[i];
			colors[i] = r.colors[i];
		}
	}
};

class SSplitResult {
public:
	struct SSplitResultData {
		SSplitResultData(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_) :
			vertices(v_), normals(n_), colors(c_), v0(v0_), v1(v1_), v2(v2_), v3(v3_), patchID(patchID_)
		{
		}
		SSplitResultData(const SSplitResultData &r) : 
			vertices(r.vertices), normals(r.normals), colors(r.colors), v0(r.v0), v1(r.v1), v2(r.v2), v3(r.v3), patchID(r.patchID)
		{}

		vector3d *vertices;
		vector3d *normals;
		vector3d *colors;
		const vector3d v0;
		const vector3d v1;
		const vector3d v2;
		const vector3d v3;
		const GeoPatchID patchID;
	};

	SSplitResult(const int32_t face_, const int32_t depth_) : face(face_), depth(depth_)
	{
	}

	void addResult(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		data.push_back(SSplitResultData(v_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
		assert(data.size()<=4);
	}

	const int32_t face;
	const int32_t depth;
	std::deque<SSplitResultData> data;

private:
	// deliberately prevent copy constructor access
	SSplitResult(const SSplitResult &r) : face(0), depth(0) {}
};

class GeoPatch {
private:
	//********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	//********************************************************************************
	class PatchJob : public PureJob
	{
	public:
		PatchJob(SSplitRequestDescription *data) : mData(data)
		{
		}

		virtual ~PatchJob()
		{
		}

		virtual void init(unsigned int *counter)
		{
			PureJob::init( counter );
		}

		virtual void job_process(void * userData,int /* userId */);    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!

		virtual void job_onFinish(void * userData, int userId);  // runs in primary thread of the context

		virtual void job_onCancel(void * userData, int userId)   // runs in primary thread of the context
		{
			PureJob::job_onCancel(userData, userId);
		}

	private:
		ScopedPtr<SSplitRequestDescription> mData;
		SSplitResult *mpResults;

		/* in patch surface coords, [0,1] */
		inline vector3d GetSpherePoint(const vector3d &v0, const vector3d &v1, const vector3d &v2, const vector3d &v3, const double x, const double y) const {
			return (v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0)).Normalized();
		}

		// Generates full-detail vertices, and also non-edge normals and colors 
		void GenerateMesh(
			vector3d *vertices, vector3d *normals, vector3d *colors, 
			const vector3d &v0,
			const vector3d &v1,
			const vector3d &v2,
			const vector3d &v3,
			const int edgeLen,
			const double fracStep) const;
	};
	ScopedPtr<PatchJob> mCurrentJob;
public:
	static const uint32_t NUM_EDGES = 4;
	static const uint32_t NUM_KIDS = NUM_EDGES;

	RefCountedPtr<GeoPatchContext> ctx;
	const vector3d v0, v1, v2, v3;
	vector3d *vertices;
	vector3d *normals;
	vector3d *colors;
	GLuint m_vbo;
	GeoPatch *kids[NUM_KIDS];
	GeoPatch *parent;
	GeoPatch *edgeFriend[NUM_EDGES]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	vector3d clipCentroid, centroid;
	double clipRadius;
	int m_depth;
	SDL_mutex *m_kidsLock;
	bool m_needUpdateVBOs;
	double m_distMult;

	const GeoPatchID mPatchID;
	bool mHasSplitRequest;
	uint8_t mCanMergeChildren;

	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs, 
		const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, 
		const int depth, const GeoPatchID &ID_);

	~GeoPatch();

	inline void UpdateVBOs() {
		m_needUpdateVBOs = (NULL!=vertices);
	}

	void _UpdateVBOs();

	/* not quite edge, since we share edge vertices so that would be
	 * fucking pointless. one position inwards. used to make edge normals
	 * for adjacent tiles */
	void GetEdgeMinusOneVerticesFlipped(const int edge, vector3d *ev) const;

	inline int GetEdgeIdxOf(const GeoPatch *e) const {
		for (int i=0; i<NUM_KIDS; i++) {if (edgeFriend[i] == e) {return i;}}
		abort();
		return -1;
	}

	void FixEdgeNormals(const int edge, const vector3d *ev);

	int GetChildIdx(const GeoPatch *child) const {
		for (int i=0; i<NUM_KIDS; i++) {
			if (kids[i] == child) return i;
		}
		abort();
		return -1;
	}

	void FixEdgeFromParentInterpolated(const int edge);

	template <int corner>
	void MakeCornerNormal(const vector3d *ev, const vector3d *ev2) {
		switch (corner) {
		case 0: {
			const vector3d &x1 = ev[ctx->edgeLen-1];
			const vector3d &x2 = vertices[1];
			const vector3d &y1 = ev2[0];
			const vector3d &y2 = vertices[ctx->edgeLen];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[0] = norm;
			// make color
			const vector3d pt = GetSpherePoint(0, 0);
		//	const double height = colors[0].x;
			const double height = geosphere->GetHeight(pt);
			colors[0] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 1: {
			const int p = ctx->edgeLen-1;
			const vector3d &x1 = vertices[p-1];
			const vector3d &x2 = ev2[0];
			const vector3d &y1 = ev[ctx->edgeLen-1];
			const vector3d &y2 = vertices[p + ctx->edgeLen];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*ctx->frac, 0);
		//	const double height = colors[p].x;
			const double height = geosphere->GetHeight(pt);
			colors[p] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 2: {
			const int p = ctx->edgeLen-1;
			const vector3d &x1 = vertices[(p-1) + p*ctx->edgeLen];
			const vector3d &x2 = ev[ctx->edgeLen-1];
			const vector3d &y1 = vertices[p + (p-1)*ctx->edgeLen];
			const vector3d &y2 = ev2[0];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p + p*ctx->edgeLen] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*ctx->frac, p*ctx->frac);
		//	const double height = colors[p + p*ctx->edgeLen].x;
			const double height = geosphere->GetHeight(pt);
			colors[p + p*ctx->edgeLen] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 3: {
			const int p = ctx->edgeLen-1;
			const vector3d &x1 = ev2[0];
			const vector3d &x2 = vertices[1 + p*ctx->edgeLen];
			const vector3d &y1 = vertices[(p-1)*ctx->edgeLen];
			const vector3d &y2 = ev[ctx->edgeLen-1];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p*ctx->edgeLen] = norm;
			// make color
			const vector3d pt = GetSpherePoint(0, p*ctx->frac);
		//	const double height = colors[p*ctx->edgeLen].x;
			const double height = geosphere->GetHeight(pt);
			colors[p*ctx->edgeLen] = geosphere->GetColor(pt, height, norm);
			}
			break;
		}
	}

	void FixCornerNormalsByEdge(const int edge, const vector3d *ev);

	void GenerateEdgeNormalsAndColors();

	// in patch surface coords, [0,1] 
	inline vector3d GetSpherePoint(const double x, const double y) const {
		return (v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0)).Normalized();
	}

	// Generates full-detail vertices, and also non-edge normals and colors
	void GenerateMesh();
	void OnEdgeFriendChanged(const int edge, GeoPatch *e);

	void NotifyEdgeFriendSplit(GeoPatch *e);

	void NotifyEdgeFriendDeleted(const GeoPatch *e);

	GeoPatch *GetEdgeFriendForKid(const int kid, const int edge) const;

	inline GLuint determineIndexbuffer() const {
		return // index buffers are ordered by edge resolution flags
			(edgeFriend[0] ? 1u : 0u) |
			(edgeFriend[1] ? 2u : 0u) |
			(edgeFriend[2] ? 4u : 0u) |
			(edgeFriend[3] ? 8u : 0u);
	}

	void Render(vector3d &campos, const Graphics::Frustum &frustum);

	inline bool canBeMerged() const {
		bool merge = true;
		if (kids[0]) {
			for (int i=0; i<NUM_KIDS; i++) {
				merge &= kids[i]->canBeMerged();
			}
		}
		merge &= !(mHasSplitRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos);
	
	void ReceiveHeightmaps(const SSplitResult *psr);
};

#endif /* _GEOPATCH_H */
