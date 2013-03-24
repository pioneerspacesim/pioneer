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

class SBaseRequest {
public:
	SBaseRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_,GeoSphere *pGeoSphere_)
		: v0(v0_), v1(v1_), v2(v2_), v3(v3_), centroid(cn), depth(depth_), 
		sysPath(sysPath_), patchID(patchID_), edgeLen(edgeLen_), fracStep(fracStep_), 
		pTerrain(pTerrain_), pGeoSphere(pGeoSphere_)
	{
	}

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }
	inline int NUMVERTICES(const int el) const { return el*el; }

	const vector3d v0, v1, v2, v3;
	const vector3d centroid;
	const uint32_t depth;
	const SystemPath sysPath;
	const GeoPatchID patchID;
	const int edgeLen;
	const double fracStep;
	Terrain *pTerrain;
	GeoSphere *pGeoSphere;	// quick hack, do not have in the final version!

protected:
	// deliberately prevent copy constructor access
	SBaseRequest(const SBaseRequest &r) : v0(0.0), v1(0.0), v2(0.0), v3(0.0), centroid(0.0), depth(0), 
		patchID(0), edgeLen(0), fracStep(0.0), pTerrain(NULL), pGeoSphere(NULL) { assert(false); }
};

class SQuadSplitRequest : public SBaseRequest {
public:
	SQuadSplitRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_,GeoSphere *pGeoSphere_)
		: SBaseRequest(v0_, v1_, v2_, v3_, cn, depth_, sysPath_, patchID_, edgeLen_, fracStep_, pTerrain_, pGeoSphere_)
	{
		const int numVerts = NUMVERTICES(edgeLen_);
		for( int i=0 ; i<4 ; ++i )
		{
			vertices[i] = new vector3d[numVerts];
			normals[i] = new vector3d[numVerts];
			colors[i] = new vector3d[numVerts];
		}
	}

	vector3d *vertices[4], *normals[4], *colors[4];

protected:
	// deliberately prevent copy constructor access
	SQuadSplitRequest(const SQuadSplitRequest &r) : SBaseRequest(r)	{ assert(false); }
};

class SSingleSplitRequest : public SBaseRequest {
public:
	SSingleSplitRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_,GeoSphere *pGeoSphere_)
		: SBaseRequest(v0_, v1_, v2_, v3_, cn, depth_, sysPath_, patchID_, edgeLen_, fracStep_, pTerrain_, pGeoSphere_)
	{
		const int numVerts = NUMVERTICES(edgeLen_);
		vertices = new vector3d[numVerts];
		normals = new vector3d[numVerts];
		colors = new vector3d[numVerts];
	}

	vector3d *vertices, *normals, *colors;

protected:
	// deliberately prevent copy constructor access
	SSingleSplitRequest(const SSingleSplitRequest &r) : SBaseRequest(r)	{ assert(false); }
};

class SBaseSplitResult {
public:
	struct SSplitResultData {
		SSplitResultData() : patchID(0) {}
		SSplitResultData(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_) :
			vertices(v_), normals(n_), colors(c_), v0(v0_), v1(v1_), v2(v2_), v3(v3_), patchID(patchID_)
		{}
		SSplitResultData(const SSplitResultData &r) : 
			vertices(r.vertices), normals(r.normals), colors(r.colors), v0(r.v0), v1(r.v1), v2(r.v2), v3(r.v3), patchID(r.patchID)
		{}

		vector3d *vertices, *normals, *colors;
		vector3d v0, v1, v2, v3;
		GeoPatchID patchID;
	};

	SBaseSplitResult(const int32_t face_, const int32_t depth_) : mFace(face_), mDepth(depth_) {}

	inline int32_t face() const { return mFace; }
	inline int32_t depth() const { return mDepth; }

	virtual void OnCancel() = 0;

protected:
	// deliberately prevent copy constructor access
	SBaseSplitResult(const SBaseSplitResult &r) : mFace(0), mDepth(0) {}

	const int32_t mFace;
	const int32_t mDepth;
};

class SQuadSplitResult : public SBaseSplitResult {
	static const int NUM_RESULT_DATA = 4;
public:
	SQuadSplitResult(const int32_t face_, const int32_t depth_) : SBaseSplitResult(face_, depth_)
	{
	}

	void addResult(const int kidIdx, vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		assert(kidIdx>=0 && kidIdx<NUM_RESULT_DATA);
		mData[kidIdx] = (SSplitResultData(v_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
	}

	inline const SSplitResultData& data(const int32_t idx) const { return mData[idx]; }

	virtual void OnCancel()
	{
		for( int i=0; i<NUM_RESULT_DATA; ++i ) {
			if( mData[i].vertices ) {delete [] mData[i].vertices;	mData[i].vertices = NULL;}
			if( mData[i].normals ) {delete [] mData[i].normals;		mData[i].normals = NULL;}
			if( mData[i].colors ) {delete [] mData[i].colors;		mData[i].colors = NULL;}
		}
	}

protected:
	// deliberately prevent copy constructor access
	SQuadSplitResult(const SQuadSplitResult &r) : SBaseSplitResult(r) {}

	SSplitResultData mData[NUM_RESULT_DATA];
};

class SSingleSplitResult : public SBaseSplitResult {
public:
	SSingleSplitResult(const int32_t face_, const int32_t depth_) : SBaseSplitResult(face_, depth_)
	{
	}

	void addResult(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		mData = (SSplitResultData(v_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
	}

	inline const SSplitResultData& data() const { return mData; }

	virtual void OnCancel()
	{
		{
			if( mData.vertices ) {delete [] mData.vertices;	mData.vertices = NULL;}
			if( mData.normals ) {delete [] mData.normals;	mData.normals = NULL;}
			if( mData.colors ) {delete [] mData.colors;		mData.colors = NULL;}
		}
	}

protected:
	// deliberately prevent copy constructor access
	SSingleSplitResult(const SSingleSplitResult &r) : SBaseSplitResult(r) {}

	SSplitResultData mData;
};

//********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
//********************************************************************************
class BasePatchJob : public PureJob
{
public:
	BasePatchJob()
	{
	}

	virtual ~BasePatchJob()
	{	
	}

	virtual void init(unsigned int *counter)
	{
		++s_numActivePatchJobs;
		PureJob::init( counter );
	}

	virtual void job_process(void * userData,int /* userId */)=0;    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void job_onFinish(void * userData, int userId)  // runs in primary thread of the context
	{
		PureJob::job_onFinish(userData, userId);
		--s_numActivePatchJobs;
	}
	virtual void job_onCancel(void * userData, int userId)   // runs in primary thread of the context
	{
		PureJob::job_onFinish(userData, userId);
		--s_numActivePatchJobs;
	}

	static uint32_t GetNumActivePatchJobs() { return s_numActivePatchJobs; };
	static void CancelAllPatchJobs() { s_abort = true; }
	static void ResetPatchJobCancel() { s_abort = false; }

protected:
	// in patch surface coords, [0,1]
	inline vector3d GetSpherePoint(const vector3d &v0, const vector3d &v1, const vector3d &v2, const vector3d &v3, const double x, const double y) const {
		return (v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0)).Normalized();
	}

	// Generates full-detail vertices, and also non-edge normals and colors 
	void GenerateMesh(vector3d *vertices, vector3d *normals, vector3d *colors, 
		const vector3d &v0, const vector3d &v1, const vector3d &v2, const vector3d &v3,
		const int edgeLen, const double fracStep, const Terrain *pTerrain) const;

	static uint32_t s_numActivePatchJobs;
	static bool s_abort;
};

//********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
//********************************************************************************
class SinglePatchJob : public BasePatchJob
{
public:
	SinglePatchJob(SSingleSplitRequest *data) : BasePatchJob(), mData(data)	{ /* empty */ }

	virtual ~SinglePatchJob()	{ /* empty */ }

	virtual void init(unsigned int *counter)
	{
		BasePatchJob::init( counter );
	}

	virtual void job_process(void * userData,int /* userId */);    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void job_onFinish(void * userData, int userId);  // runs in primary thread of the context
	virtual void job_onCancel(void * userData, int userId);   // runs in primary thread of the context

private:
	ScopedPtr<SSingleSplitRequest> mData;
	SSingleSplitResult *mpResults;
};

//********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
//********************************************************************************
class QuadPatchJob : public BasePatchJob
{
public:
	QuadPatchJob(SQuadSplitRequest *data) : BasePatchJob(), mData(data) { /* empty */ }

	virtual ~QuadPatchJob()	{ /* empty */ }

	virtual void init(unsigned int *counter)
	{
		BasePatchJob::init( counter );
	}

	virtual void job_process(void * userData,int /* userId */);    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void job_onFinish(void * userData, int userId);  // runs in primary thread of the context
	virtual void job_onCancel(void * userData, int userId);   // runs in primary thread of the context

private:
	ScopedPtr<SQuadSplitRequest> mData;
	SQuadSplitResult *mpResults;
};

class GeoPatch {
public:
	
	ScopedPtr<BasePatchJob>		mCurrentJob;
	JOB_SWARM::SwarmJob*		mpSwarmJob;
public:
	static const uint32_t NUM_EDGES = 4;
	static const uint32_t NUM_KIDS = NUM_EDGES;

	RefCountedPtr<GeoPatchContext> ctx;
	const vector3d v0, v1, v2, v3;
	ScopedArray<vector3d> vertices;
	ScopedArray<vector3d> normals;
	ScopedArray<vector3d> colors;
	GLuint m_vbo;
	ScopedPtr<GeoPatch> kids[NUM_KIDS];
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
	bool mHasJobRequest;
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
		merge &= !(mHasJobRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos);
	
	void RequestSinglePatch();
	void ReceiveHeightmaps(const SQuadSplitResult *psr);
	void ReceiveHeightmap(const SSingleSplitResult *psr);
};

#endif /* _GEOPATCH_H */
