// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASGIANTJOBS_H
#define _GASGIANTJOBS_H

#include <SDL_stdinc.h>

#include "JobQueue.h"
#include "galaxy/SystemPath.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderState.h"
#include "profiler/Profiler.h"
#include "terrain/Terrain.h"
#include "vector3.h"

#include <deque>

namespace Graphics {
	class Renderer;
} // namespace Graphics

namespace GasGiantJobs {
	//#define DUMP_PARAMS 1

	enum GasGiantTexture {
		GEN_JUPITER_TEXTURE = 0,
		GEN_SATURN_TEXTURE,
		GEN_SATURN2_TEXTURE,
		// technically Ice Giants not Gas Giants...
		GEN_NEPTUNE_TEXTURE,
		GEN_NEPTUNE2_TEXTURE,
		GEN_URANUS_TEXTURE
	};

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d(1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1, -1, 1)).Normalized();
	static const vector3d p4 = (vector3d(1, -1, 1)).Normalized();
	static const vector3d p5 = (vector3d(1, 1, -1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1, -1)).Normalized();
	static const vector3d p7 = (vector3d(-1, -1, -1)).Normalized();
	static const vector3d p8 = (vector3d(1, -1, -1)).Normalized();

	const vector3d &GetPatchFaces(const Uint32 patch, const Uint32 face);

	// ********************************************************************************
	// a quad with reversed winding
	class GenFaceQuad {
	public:
		GenFaceQuad(Graphics::Renderer *r, const vector2f &size, const Uint32 GGQuality);
		virtual void Draw(Graphics::Renderer *r);

		void SetMaterial(Graphics::Material *mat)
		{
			assert(mat);
			m_material.reset(mat);
		}
		Graphics::Material *GetMaterial() const { return m_material.get(); }

	private:
		std::unique_ptr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::MeshObject> m_quadMesh;
	};

	// ********************************************************************************
	class SGPUGenRequest {
	public:
		SGPUGenRequest(const SystemPath &sysPath_, const Sint32 uvDIMs_, Terrain *pTerrain_, const float planetRadius_, const float hueAdjust_, GenFaceQuad *pQuad_, Graphics::Texture *pTex_);

		inline Sint32 UVDims() const { return uvDIMs; }
		Graphics::Texture *Texture() const { return m_texture.Get(); }
		GenFaceQuad *Quad() const { return pQuad.get(); }
		const SystemPath &SysPath() const { return sysPath; }
		void SetupMaterialParams(const int face);

	protected:
		// deliberately prevent copy constructor access
		SGPUGenRequest(const SGPUGenRequest &r) = delete;

		inline Sint32 NumTexels() const { return uvDIMs * uvDIMs; }

		// this is created with the request and are given to the resulting patches
		RefCountedPtr<Graphics::Texture> m_texture;

		const SystemPath sysPath;
		const Sint32 uvDIMs;
		Terrain *pTerrain;
		const float planetRadius;
		const float hueAdjust;
		std::unique_ptr<GenFaceQuad> pQuad;
	};

	// ********************************************************************************
	class SGPUGenResult {
	public:
		struct SGPUGenData {
			SGPUGenData() {}
			SGPUGenData(Graphics::Texture *t_, Sint32 uvDims_) :
				texture(t_),
				uvDims(uvDims_) {}
			RefCountedPtr<Graphics::Texture> texture;
			Sint32 uvDims;
		};

		SGPUGenResult() {}

		void addResult(Graphics::Texture *t_, Sint32 uvDims_);

		inline const SGPUGenData &data() const { return mData; }

		void OnCancel();

	protected:
		// deliberately prevent copy constructor access
		SGPUGenResult(const SGPUGenResult &r) = delete;

		SGPUGenData mData;
	};

	// ********************************************************************************
	// Overloaded JobGPU class to handle generating the mesh for each patch
	// ********************************************************************************
	class SingleGPUGenJob : public Job {
	public:
		SingleGPUGenJob(SGPUGenRequest *data);
		virtual ~SingleGPUGenJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		SingleGPUGenJob() {}
		// deliberately prevent copy constructor access
		SingleGPUGenJob(const SingleGPUGenJob &r) = delete;

		std::unique_ptr<SGPUGenRequest> mData;
		SGPUGenResult *mpResults;
	};
} // namespace GasGiantJobs

#endif /* _GASGIANTJOBS_H */
