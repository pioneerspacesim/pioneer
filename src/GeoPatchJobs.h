// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCHJOBS_H
#define _GEOPATCHJOBS_H

#include <SDL_stdinc.h>

#include "Color.h"
#include "GeoPatchID.h"
#include "JobQueue.h"
#include "vector3.h"
#include "terrain/Terrain.h"

class GeoSphere;

#define BORDER_SIZE 1

class SBaseRequest {
public:
	SBaseRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_) :
		v0(v0_),
		v1(v1_),
		v2(v2_),
		v3(v3_),
		centroid(cn),
		depth(depth_),
		sysPath(sysPath_),
		patchID(patchID_),
		edgeLen(edgeLen_),
		fracStep(fracStep_),
		pTerrain(pTerrain_)
	{
	}

	inline int NUMVERTICES(const int el) const { return el * el; }

	const vector3d v0, v1, v2, v3;
	const vector3d centroid;
	const uint32_t depth;
	const SystemPath sysPath;
	const GeoPatchID patchID;
	const int edgeLen;
	const double fracStep;
	RefCountedPtr<Terrain> pTerrain;

protected:
	// deliberately prevent copy constructor access
	SBaseRequest(const SBaseRequest &r) = delete;
};

class SQuadSplitRequest : public SBaseRequest {
public:
	SQuadSplitRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_) :
		SBaseRequest(v0_, v1_, v2_, v3_, cn, depth_, sysPath_, patchID_, edgeLen_, fracStep_, pTerrain_)
	{
		const int numVerts = NUMVERTICES(edgeLen_);
		for (int i = 0; i < 4; ++i) {
			heights[i] = new double[numVerts];
			normals[i] = new vector3f[numVerts];
			colors[i] = new Color3ub[numVerts];
		}
		const int numBorderedVerts = NUMVERTICES((edgeLen_ * 2) + (BORDER_SIZE * 2) - 1);
		borderHeights.reset(new double[numBorderedVerts]);
		borderVertexs.reset(new vector3d[numBorderedVerts]);
	}

	// Generates full-detail vertices, and also non-edge normals and colors
	void GenerateBorderedData() const;

	void GenerateSubPatchData(const int quadrantIndex,
		const vector3d &v0, const vector3d &v1, const vector3d &v2, const vector3d &v3,
		const int edgeLen, const int xoff, const int yoff, const int borderedEdgeLen) const;

	// these are created with the request and are given to the resulting patches
	vector3f *normals[4];
	Color3ub *colors[4];
	double *heights[4];

	// these are created with the request but are destroyed when the request is finished
	std::unique_ptr<double[]> borderHeights;
	std::unique_ptr<vector3d[]> borderVertexs;

protected:
	// deliberately prevent copy constructor access
	SQuadSplitRequest(const SQuadSplitRequest &r) = delete;
};

class SSingleSplitRequest : public SBaseRequest {
public:
	SSingleSplitRequest(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const vector3d &cn,
		const uint32_t depth_, const SystemPath &sysPath_, const GeoPatchID &patchID_, const int edgeLen_, const double fracStep_,
		Terrain *pTerrain_) :
		SBaseRequest(v0_, v1_, v2_, v3_, cn, depth_, sysPath_, patchID_, edgeLen_, fracStep_, pTerrain_)
	{
		const int numVerts = NUMVERTICES(edgeLen_);
		heights = new double[numVerts];
		normals = new vector3f[numVerts];
		colors = new Color3ub[numVerts];

		const int numBorderedVerts = NUMVERTICES(edgeLen_ + (BORDER_SIZE * 2));
		borderHeights.reset(new double[numBorderedVerts]);
		borderVertexs.reset(new vector3d[numBorderedVerts]);
	}

	// Generates full-detail vertices, and also non-edge normals and colors
	void GenerateMesh() const;

	// these are created with the request and are given to the resulting patches
	vector3f *normals;
	Color3ub *colors;
	double *heights;

	// these are created with the request but are destroyed when the request is finished
	std::unique_ptr<double[]> borderHeights;
	std::unique_ptr<vector3d[]> borderVertexs;

protected:
	// deliberately prevent copy constructor access
	SSingleSplitRequest(const SSingleSplitRequest &r) = delete;
};

struct SSplitResultData {
	SSplitResultData() :
		patchID(0) {}
	SSplitResultData(double *heights_, vector3f *n_, Color3ub *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_) :
		heights(heights_),
		normals(n_),
		colors(c_),
		v0(v0_),
		v1(v1_),
		v2(v2_),
		v3(v3_),
		patchID(patchID_)
	{}

	double *heights;
	vector3f *normals;
	Color3ub *colors;
	vector3d v0, v1, v2, v3;
	GeoPatchID patchID;
};

class SBaseSplitResult {
public:
	SBaseSplitResult(const int32_t face_, const int32_t depth_) :
		mFace(face_),
		mDepth(depth_) {}
	virtual ~SBaseSplitResult() {}

	inline int32_t face() const { return mFace; }
	inline int32_t depth() const { return mDepth; }

	virtual void OnCancel() = 0;

protected:
	// deliberately prevent copy constructor access
	SBaseSplitResult(const SBaseSplitResult &r) :
		mFace(0),
		mDepth(0) {}

	const int32_t mFace;
	const int32_t mDepth;
};

class SQuadSplitResult : public SBaseSplitResult {
	static const int NUM_RESULT_DATA = 4;

public:
	SQuadSplitResult(const int32_t face_, const int32_t depth_) :
		SBaseSplitResult(face_, depth_)
	{
	}

	void addResult(const int kidIdx, double *h_, vector3f *n_, Color3ub *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		assert(kidIdx >= 0 && kidIdx < NUM_RESULT_DATA);
		mData[kidIdx] = (SSplitResultData(h_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
	}

	inline const SSplitResultData &data(const int32_t idx) const { return mData[idx]; }

	virtual void OnCancel()
	{
		for (int i = 0; i < NUM_RESULT_DATA; ++i) {
			if (mData[i].heights) {
				delete[] mData[i].heights;
				mData[i].heights = NULL;
			}
			if (mData[i].normals) {
				delete[] mData[i].normals;
				mData[i].normals = NULL;
			}
			if (mData[i].colors) {
				delete[] mData[i].colors;
				mData[i].colors = NULL;
			}
		}
	}

protected:
	// deliberately prevent copy constructor access
	SQuadSplitResult(const SQuadSplitResult &r) :
		SBaseSplitResult(r) {}

	SSplitResultData mData[NUM_RESULT_DATA];
};

class SSingleSplitResult : public SBaseSplitResult {
public:
	SSingleSplitResult(const int32_t face_, const int32_t depth_) :
		SBaseSplitResult(face_, depth_)
	{
	}

	void addResult(double *h_, vector3f *n_, Color3ub *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		mData = (SSplitResultData(h_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
	}

	inline const SSplitResultData &data() const { return mData; }

	virtual void OnCancel()
	{
		{
			if (mData.heights) {
				delete[] mData.heights;
				mData.heights = NULL;
			}
			if (mData.normals) {
				delete[] mData.normals;
				mData.normals = NULL;
			}
			if (mData.colors) {
				delete[] mData.colors;
				mData.colors = NULL;
			}
		}
	}

protected:
	// deliberately prevent copy constructor access
	SSingleSplitResult(const SSingleSplitResult &r) :
		SBaseSplitResult(r) {}

	SSplitResultData mData;
};

class GeoPatch;

// ********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
// ********************************************************************************
class BasePatchJob : public Job {
public:
	BasePatchJob() {}
	virtual void OnRun() {} // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void OnFinish() {}
	virtual void OnCancel() {}
};

// ********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
// ********************************************************************************
class SinglePatchJob : public BasePatchJob {
public:
	SinglePatchJob(SSingleSplitRequest *data) :
		mData(data),
		mpResults(NULL)
	{ /* empty */
	}
	~SinglePatchJob();

	virtual void OnRun(); // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void OnFinish(); // runs in primary thread of the context

private:
	std::unique_ptr<SSingleSplitRequest> mData;
	SSingleSplitResult *mpResults;
};

// ********************************************************************************
// Overloaded PureJob class to handle generating the mesh for each patch
// ********************************************************************************
class QuadPatchJob : public BasePatchJob {
public:
	QuadPatchJob(SQuadSplitRequest *data) :
		mData(data),
		mpResults(NULL)
	{ /* empty */
	}
	~QuadPatchJob();

	virtual void OnRun(); // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void OnFinish(); // runs in primary thread of the context

private:
	std::unique_ptr<SQuadSplitRequest> mData;
	SQuadSplitResult *mpResults;
};

#endif /* _GEOPATCHJOBS_H */
