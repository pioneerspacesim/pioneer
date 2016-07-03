// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CLOUDJOBS_H
#define _CLOUDJOBS_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "graphics/opengl/GenCloudSphereMaterial.h"
#include "graphics/TextureBuilder.h"
//#include "terrain/Terrain.h"
//#include "BaseSphere.h"
//#include "GeoSphere.h"
#include "JobQueue.h"

#include <deque>
#include <functional>

namespace CloudJobs
{
	//#define DUMP_PARAMS 1
	// fwd declarations
	class CloudCPUGenResult;
	class CloudGPUGenResult;

	class CloudCPUGenRequest {
	public:
		CloudCPUGenRequest(const SystemPath &sysPath_, const Sint32 face_, bool (*callback)(const SystemPath &, CloudCPUGenResult *));

		// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		// Use only data local to this object
		void OnRun();

		Sint32 Face() const { return face; }
		//inline Sint32 UVDims() const { return uvDIMs; }
		Color* Colors() const { return colors; }
		const SystemPath& SysPath() const { return sysPath; }

		bool InvokeCallback(const SystemPath &sp, CloudCPUGenResult *stfr)
		{
			if(m_callback)
				return m_callback(sp, stfr);
			return false;
		}

	protected:
		// deliberately prevent copy constructor access
		CloudCPUGenRequest(const CloudCPUGenRequest &r) = delete;

		// in patch surface coords, [0,1]
		inline vector3d GetSpherePoint(const double x, const double y) const {
			return (corners[0] + x*(1.0 - y)*(corners[1] - corners[0]) + x*y*(corners[2] - corners[0]) + (1.0 - x)*y*(corners[3] - corners[0])).Normalized();
		}

		// these are created with the request and are given to the resulting patches
		Color *colors;

		const vector3d *corners;
		const SystemPath sysPath;
		const Sint32 face;

		std::function<bool(const SystemPath &, CloudCPUGenResult *)> m_callback;
	};

	class CloudCPUGenResult {
	public:
		struct STextureFaceData {
			STextureFaceData() {}
			STextureFaceData(Color *c_, Sint32 uvDims_) : colors(c_), uvDims(uvDims_) {}
			STextureFaceData(const STextureFaceData &r) : colors(r.colors), uvDims(r.uvDims) {}
			Color *colors;
			Sint32 uvDims;
		};

		CloudCPUGenResult(const int32_t face_) : mFace(face_) {}

		void AddResult(Color *c_, Sint32 uvDims_) {
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
		CloudCPUGenResult(const CloudCPUGenResult &r) = delete;

		const int32_t mFace;
		STextureFaceData mData;
	};

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	class CPUCloudSphereJob : public Job
	{
	public:
		CPUCloudSphereJob(CloudCPUGenRequest *data) : mData(data), mpResults(nullptr) { /* empty */ }
		virtual ~CPUCloudSphereJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		// deliberately prevent copy constructor access
		CPUCloudSphereJob(const CPUCloudSphereJob &r);

		std::unique_ptr<CloudCPUGenRequest> mData;
		CloudCPUGenResult *mpResults;
	};

	// ********************************************************************************
	// a quad with reversed winding
	class GenFaceQuad {
	public:
		GenFaceQuad();
		void Draw();

		void SetMaterial(Graphics::Material *mat) { assert(mat); m_material.reset(mat); }
		Graphics::Material* GetMaterial() const { return m_material.get(); }
	private:
		std::unique_ptr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
	};

	// ********************************************************************************
	class CloudGPUGenRequest {
	public:
		CloudGPUGenRequest(const SystemPath &sysPath_, const float planetRadius_, bool (*callback)(const SystemPath &, CloudGPUGenResult *));

		Graphics::Texture* Texture() const { return m_texture.Get(); }
		GenFaceQuad& Quad() { return m_quad; }
		const SystemPath& SysPath() const { return sysPath; }
		void SetupMaterialParams(const int face);

		bool InvokeCallback(const SystemPath &, CloudGPUGenResult *);

	protected:
		// deliberately prevent copy constructor access
		CloudGPUGenRequest(const CloudGPUGenRequest &r) = delete;

		// this is created with the request and are given to the resulting patches
		RefCountedPtr<Graphics::Texture> m_texture;

		const SystemPath sysPath;
		const float planetRadius;
		GenFaceQuad m_quad;
		Graphics::GenCloudSphereMaterialParameters m_specialParams;

		std::function<bool(const SystemPath &, CloudGPUGenResult *)> m_callback;
	};

	// ********************************************************************************
	class CloudGPUGenResult {
	public:
		struct CloudGPUGenData {
			CloudGPUGenData() {}
			CloudGPUGenData(Graphics::Texture *t_, Sint32 uvDims_) : texture(t_), uvDims(uvDims_) {}
			CloudGPUGenData(const CloudGPUGenData &r) : texture(r.texture), uvDims(r.uvDims) {}
			RefCountedPtr<Graphics::Texture> texture;
			Sint32 uvDims;
		};

		CloudGPUGenResult() {}

		void AddResult(Graphics::Texture *t_, Sint32 uvDims_);

		inline const CloudGPUGenData& data() const { return mData; }

		void OnCancel();

	protected:
		// deliberately prevent copy constructor access
		CloudGPUGenResult(const CloudGPUGenResult &r) = delete;

		CloudGPUGenData mData;
	};

	// ********************************************************************************
	// Overloaded JobGPU class to handle generating the mesh for each patch
	// ********************************************************************************
	class GPUCloudSphereJob : public Job
	{
	public:
		GPUCloudSphereJob(CloudGPUGenRequest *data);
		virtual ~GPUCloudSphereJob();

		virtual void OnRun();
		virtual void OnFinish();
		virtual void OnCancel() {}

	private:
		GPUCloudSphereJob() {}
		// deliberately prevent copy constructor access
		GPUCloudSphereJob(const GPUCloudSphereJob &r);

		std::unique_ptr<CloudGPUGenRequest> mData;
		CloudGPUGenResult *mpResults;
	};
};


#endif /* _GASGIANTJOBS_H */
