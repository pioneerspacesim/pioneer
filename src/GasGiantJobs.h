// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASGIANTJOBS_H
#define _GASGIANTJOBS_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "graphics/opengl/GenGasGiantColourMaterial.h"
#include "graphics/TextureBuilder.h"
#include "terrain/Terrain.h"
#include "BaseSphere.h"
#include "GeoSphere.h"
#include "JobQueue.h"

#include <deque>

namespace GasGiantJobs
{
	//#define DUMP_PARAMS 1

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d(1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1, -1, 1)).Normalized();
	static const vector3d p4 = (vector3d(1, -1, 1)).Normalized();
	static const vector3d p5 = (vector3d(1, 1, -1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1, -1)).Normalized();
	static const vector3d p7 = (vector3d(-1, -1, -1)).Normalized();
	static const vector3d p8 = (vector3d(1, -1, -1)).Normalized();

	static const vector3d s_patchFaces[NUM_PATCHES][4] =
	{
		{ p5, p1, p4, p8 }, // +x
		{ p2, p6, p7, p3 }, // -x

		{ p2, p1, p5, p6 }, // +y
		{ p7, p8, p4, p3 }, // -y

		{ p6, p5, p8, p7 }, // +z - NB: these are actually reversed!
		{ p1, p2, p3, p4 }  // -z
	};

	class STextureFaceRequest {
	public:
		STextureFaceRequest(const vector3d *v_, const SystemPath &sysPath_, const Sint32 face_, const Sint32 uvDIMs_, Terrain *pTerrain_);

		// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		// Use only data local to this object
		void OnRun();

		Sint32 Face() const { return face; }
		inline Sint32 UVDims() const { return uvDIMs; }
		Color* Colors() const { return colors; }
		const SystemPath& SysPath() const { return sysPath; }
#ifdef DUMP_PARAMS
		void DumpParams()
		{
			//
			Output("--face-%d------------\n", face);
			const vector3d *v = corners;
			for (int i = 0; i < 4; i++)
				Output("v(%.4f,%.4f,%.4f)\n", v[i].x, v[i].y, v[i].z);

			Output("fracStep = %.4f\n", 1.0f / float(uvDIMs));
			Output("time = 0.0f\n");

			for (Uint32 i = 0; i<3; i++) {
				const fracdef_t &fd = pTerrain->GetFracDef(i);
				Output("frequency[%u] = %.4f\n", i, fd.frequency);
			}
			// XXX omg hacking galore
		}
#endif

	protected:
		// deliberately prevent copy constructor access
		STextureFaceRequest(const STextureFaceRequest &r);

		inline Sint32 NumTexels() const { return uvDIMs*uvDIMs; }

		// in patch surface coords, [0,1]
		inline vector3d GetSpherePoint(const double x, const double y) const {
			return (corners[0] + x*(1.0 - y)*(corners[1] - corners[0]) + x*y*(corners[2] - corners[0]) + (1.0 - x)*y*(corners[3] - corners[0])).Normalized();
		}

		// these are created with the request and are given to the resulting patches
		Color *colors;

		const vector3d *corners;
		const SystemPath sysPath;
		const Sint32 face;
		const Sint32 uvDIMs;
		RefCountedPtr<Terrain> pTerrain;
	};

	class STextureFaceResult {
	public:
		struct STextureFaceData {
			STextureFaceData() {}
			STextureFaceData(Color *c_, Sint32 uvDims_) : colors(c_), uvDims(uvDims_) {}
			STextureFaceData(const STextureFaceData &r) : colors(r.colors), uvDims(r.uvDims) {}
			Color *colors;
			Sint32 uvDims;
		};

		STextureFaceResult(const int32_t face_) : mFace(face_) {}

		void addResult(Color *c_, Sint32 uvDims_) {
			PROFILE_SCOPED()
			mData = STextureFaceData(c_, uvDims_);
		}

		inline const STextureFaceData& data() const { return mData; }
		inline int32_t face() const { return mFace; }

		void OnCancel()	{
			if (mData.colors) { delete[] mData.colors; mData.colors = NULL; }
		}

	protected:
		// deliberately prevent copy constructor access
		STextureFaceResult(const STextureFaceResult &r);

		const int32_t mFace;
		STextureFaceData mData;
	};

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	class SingleTextureFaceJob : public Job
	{
	public:
		SingleTextureFaceJob(STextureFaceRequest *data) : mData(data), mpResults(nullptr) { /* empty */ }
		virtual ~SingleTextureFaceJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		// deliberately prevent copy constructor access
		SingleTextureFaceJob(const SingleTextureFaceJob &r);

		std::unique_ptr<STextureFaceRequest> mData;
		STextureFaceResult *mpResults;
	};

	// ********************************************************************************
	// a quad with reversed winding
	class GenFaceQuad {
	public:
		GenFaceQuad(Graphics::Renderer *r, const vector2f &size, Graphics::RenderState *state, const Uint32 GGQuality);
		virtual void Draw(Graphics::Renderer *r);

		void SetMaterial(Graphics::Material *mat) { assert(mat); m_material.reset(mat); }
		Graphics::Material* GetMaterial() const { return m_material.get(); }
	private:
		std::unique_ptr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
		Graphics::RenderState *m_renderState;
	};

	// ********************************************************************************
	class SGPUGenRequest {
	public:
		SGPUGenRequest(const SystemPath &sysPath_, const Sint32 uvDIMs_, Terrain *pTerrain_, const float planetRadius_, GenFaceQuad* pQuad_, Graphics::Texture *pTex_);

		inline Sint32 UVDims() const { return uvDIMs; }
		Graphics::Texture* Texture() const { return m_texture.Get(); }
		GenFaceQuad* Quad() const { return pQuad; }
		const SystemPath& SysPath() const { return sysPath; }

		void SetupMaterialParams(const int face)
		{
			PROFILE_SCOPED()
			m_specialParams.v = &s_patchFaces[face][0];
			m_specialParams.fracStep = 1.0f / float(uvDIMs);
			m_specialParams.planetRadius = planetRadius;
			m_specialParams.time = 0.0f;
			
			for(Uint32 i=0; i<3; i++) {
				m_specialParams.frequency[i] = (float)pTerrain->GetFracDef(i).frequency;
			}
			pQuad->GetMaterial()->specialParameter0 = &m_specialParams;
		}

#ifdef DUMP_PARAMS
		void DumpParams(const int face)
		{
			//
			Output("--face-%d------------\n", face);
			const vector3d *v = &s_patchFaces[face][0];
			for (int i = 0; i < 4;i++)
				Output("v(%.4f,%.4f,%.4f)\n", v[i].x, v[i].y, v[i].z);

			Output("fracStep = %.4f\n", 1.0f / float(uvDIMs));
			Output("time = 0.0f\n");

			for (Uint32 i = 0; i<10; i++) {
				const fracdef_t &fd = pTerrain->GetFracDef(i);
				Output("frequency[%u] = %.4f\n", i, fd.frequency);
			}
			// XXX omg hacking galore
		}
#endif

	protected:
		// deliberately prevent copy constructor access
		SGPUGenRequest(const SGPUGenRequest &r);

		inline Sint32 NumTexels() const { return uvDIMs*uvDIMs; }

		// this is created with the request and are given to the resulting patches
		RefCountedPtr<Graphics::Texture> m_texture;

		const SystemPath sysPath;
		const Sint32 uvDIMs;
		Terrain *pTerrain;
		const float planetRadius;
		GenFaceQuad* pQuad;
		Graphics::GenGasGiantColourMaterialParameters m_specialParams;
	};

	// ********************************************************************************
	class SGPUGenResult {
	public:
		struct SGPUGenData {
			SGPUGenData() {}
			SGPUGenData(Graphics::Texture *t_, Sint32 uvDims_) : texture(t_), uvDims(uvDims_) {}
			SGPUGenData(const SGPUGenData &r) : texture(r.texture), uvDims(r.uvDims) {}
			RefCountedPtr<Graphics::Texture> texture;
			Sint32 uvDims;
		};

		SGPUGenResult() {}

		void addResult(Graphics::Texture *t_, Sint32 uvDims_);

		inline const SGPUGenData& data() const { return mData; }

		void OnCancel();

	protected:
		// deliberately prevent copy constructor access
		SGPUGenResult(const SGPUGenResult &r);

		SGPUGenData mData;
	};

	// ********************************************************************************
	// Overloaded JobGPU class to handle generating the mesh for each patch
	// ********************************************************************************
	class SingleGPUGenJob : public Job
	{
	public:
		SingleGPUGenJob(SGPUGenRequest *data);
		virtual ~SingleGPUGenJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		SingleGPUGenJob() {}
		// deliberately prevent copy constructor access
		SingleGPUGenJob(const SingleGPUGenJob &r);

		std::unique_ptr<SGPUGenRequest> mData;
		SGPUGenResult *mpResults;
	};
};


#endif /* _GASGIANTJOBS_H */
