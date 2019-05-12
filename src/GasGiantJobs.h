// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASGIANTJOBS_H
#define _GASGIANTJOBS_H

#include <SDL_stdinc.h>

#include "JobQueue.h"
#include "graphics/Material.h"
#include "graphics/opengl/GenGasGiantColourMaterial.h"
#include "graphics/VertexBuffer.h"
#include "terrain/Terrain.h"
#include "vector3.h"

#include <deque>

namespace Graphics {
	class Renderer;
	class RenderState;
}

namespace GasGiantJobs {
	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d(1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1, -1, 1)).Normalized();
	static const vector3d p4 = (vector3d(1, -1, 1)).Normalized();
	static const vector3d p5 = (vector3d(1, 1, -1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1, -1)).Normalized();
	static const vector3d p7 = (vector3d(-1, -1, -1)).Normalized();
	static const vector3d p8 = (vector3d(1, -1, -1)).Normalized();

	//const vector3d &GetPatchFaces(const Uint32 patch, const Uint32 face);
	void InstantTextureGenerator(const double fracStep, const Terrain *pTerrain, const Uint32 GGQuality, const float hueAdjust, Graphics::Texture *texOut);

	class STextureFaceRequest {
	public:
		STextureFaceRequest(const SystemPath &sysPath_, const Sint32 face_, const Sint32 uvDIMs_, const Terrain *pTerrain_, const Uint32 GGQuality, const float hueAdjust_);

		// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		// Use only data local to this object
		void OnRun();

		Sint32 Face() const { return face; }
		inline Sint32 UVDims() const { return uvDIMs; }
		Color *Colors() const { return colors; }
		const SystemPath &SysPath() const { return sysPath; }

	protected:
		// deliberately prevent copy constructor access
		STextureFaceRequest(const STextureFaceRequest &r) = delete;

		inline Sint32 NumTexels() const { return uvDIMs * uvDIMs; }

		// these are created with the request and are given to the resulting patches
		Color *colors;

		const vector3d *corners;
		const SystemPath sysPath;
		const Sint32 face;
		const Sint32 uvDIMs;
		RefCountedPtr<const Terrain> pTerrain;
		const Graphics::OGL::GasGiantQuality ggQuality;
		const float hueAdjust;
	};

	class STextureFaceResult {
	public:
		struct STextureFaceData {
			STextureFaceData() {}
			STextureFaceData(Color *c_, Sint32 uvDims_) :
				colors(c_),
				uvDims(uvDims_) {}
			Color *colors;
			Sint32 uvDims;
		};

		STextureFaceResult(const int32_t face_) :
			mFace(face_) {}

		void addResult(Color *c_, Sint32 uvDims_)
		{
			PROFILE_SCOPED()
			mData = STextureFaceData(c_, uvDims_);
		}

		inline const STextureFaceData &data() const { return mData; }
		inline int32_t face() const { return mFace; }

		void OnCancel()
		{
			if (mData.colors) {
				delete[] mData.colors;
				mData.colors = NULL;
			}
		}

	protected:
		// deliberately prevent copy constructor access
		STextureFaceResult(const STextureFaceResult &r) = delete;

		const int32_t mFace;
		STextureFaceData mData;
	};

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	class SingleTextureFaceJob : public Job {
	public:
		SingleTextureFaceJob(STextureFaceRequest *data) :
			mData(data),
			mpResults(nullptr)
		{ /* empty */
		}
		virtual ~SingleTextureFaceJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		// deliberately prevent copy constructor access
		SingleTextureFaceJob(const SingleTextureFaceJob &r) = delete;

		std::unique_ptr<STextureFaceRequest> mData;
		STextureFaceResult *mpResults;
	};

	// ********************************************************************************
	// a quad with reversed winding
	class GenFaceQuad {
	public:
		GenFaceQuad(Graphics::Renderer *r, const vector2f &size, Graphics::RenderState *state, const Uint32 GGQuality);
		virtual void Draw(Graphics::Renderer *r);

		void SetMaterial(Graphics::Material *mat)
		{
			assert(mat);
			m_material.reset(mat);
		}
		Graphics::Material *GetMaterial() const { return m_material.get(); }

	private:
		std::unique_ptr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
		Graphics::RenderState *m_renderState;
	};

	// ********************************************************************************
	class SGPUGenRequest {
	public:
		SGPUGenRequest(const SystemPath &sysPath_, const Sint32 uvDIMs_, Terrain *pTerrain_, const float planetRadius_, const float hueAdjust_, GenFaceQuad *pQuad_, Graphics::Texture *pTex_);

		inline Sint32 UVDims() const { return uvDIMs; }
		Graphics::Texture *Texture() const { return m_texture.Get(); }
		GenFaceQuad *Quad() const { return pQuad; }
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
		GenFaceQuad *pQuad;
		Graphics::GenGasGiantColourMaterialParameters m_specialParams;
	};

	// ********************************************************************************
	class SGPUGenResult {
	public:
		struct SGPUGenData {
			SGPUGenData(Graphics::Texture *t_, Sint32 uvDims_) :
				texture(t_),
				uvDims(uvDims_) {}
			RefCountedPtr<Graphics::Texture> texture;
			Sint32 uvDims;
		};

		SGPUGenResult(Graphics::Texture *t_, Sint32 uvDims_);

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
		SingleGPUGenJob() = delete;
		// deliberately prevent copy constructor access
		SingleGPUGenJob(const SingleGPUGenJob &r) = delete;

		std::unique_ptr<SGPUGenRequest> mData;
		SGPUGenResult *mpResults;
	};
} // namespace GasGiantJobs

#endif /* _GASGIANTJOBS_H */
