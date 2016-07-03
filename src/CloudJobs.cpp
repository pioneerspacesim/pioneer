// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "CloudJobs.h"
#include "perlin.h"
#include "Pi.h"
#include "Game.h"
#include "GeoSphere.h"
#include "IniConfig.h"
#include "FileSystem.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/opengl/GenGasGiantColourMaterial.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "graphics/Types.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>

namespace CloudJobs
{
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

	const float CloudCover = 0.25;
	const float CloudSharpness = 0.25;
	const float nScale = 1.4; // Uniform?
	const float Density = 0.02;
	const float ESun = 1.0;

	
	// ********************************************************************************
	// 
	// ********************************************************************************
	class GPUCloudSphereContext {
	private:
		static GPUCloudSphereContext	*m_instance;
		static Graphics::RenderTarget	*s_renderTarget;
		static Graphics::RenderState	*s_quadRenderState;
		static Uint32 m_textureSizeCpu;
		static Uint32 m_textureSizeGpu;

		GPUCloudSphereContext() 
		{
			assert(nullptr==m_instance);
			if(m_instance)
			{
				delete m_instance;
			}

			m_instance = this;
			s_renderTarget = nullptr;
			s_quadRenderState = nullptr;
			Init();
		}
		
		static void Init()
		{
			IniConfig cfg;
			cfg.Read(FileSystem::gameDataFiles, "configs/Cloud.ini");
			// NB: limit the ranges of all values loaded from the file
			// NB: round to the nearest power of 2 for all texture sizes
			m_textureSizeCpu		= ceil_pow2(Clamp(cfg.Int("texture_size_cpu", 512), 128, 4096));
			m_textureSizeGpu		= ceil_pow2(Clamp(cfg.Int("texture_size_gpu", 1024), 128, 4096));

			CreateRenderTarget(m_textureSizeGpu, m_textureSizeGpu);
		}

	public:
		static GPUCloudSphereContext* Instance() 
		{
			return (m_instance) ? m_instance : (new GPUCloudSphereContext());
		}

		//static
		static void CreateRenderTarget(const Uint16 width, const Uint16 height) {
			/*	@fluffyfreak here's a rendertarget implementation you can use for oculusing and other things. It's pretty simple:
				 - fill out a RenderTargetDesc struct and call Renderer::CreateRenderTarget
				 - pass target to Renderer::SetRenderTarget to start rendering to texture
				 - set up viewport, clear etc, then draw as usual
				 - SetRenderTarget(0) to resume render to screen
				 - you can access the attached texture with GetColorTexture to use it with a material
				You can reuse the same target with multiple textures.
				In that case, leave the color format to NONE so the initial texture is not created, then use SetColorTexture to attach your own.
			*/
			Graphics::RenderStateDesc rsd;
			rsd.depthTest  = false;
			rsd.depthWrite = false;
			rsd.blendMode = Graphics::BLEND_ALPHA;
			s_quadRenderState = Pi::renderer->CreateRenderState(rsd);

			// Complete the RT description so we can request a buffer.
			// NB: we don't want it to create use a texture because we share it with the textured quad created above.
			Graphics::RenderTargetDesc rtDesc(
				width,
				height,
				Graphics::TEXTURE_NONE,		// don't create a texture
				Graphics::TEXTURE_NONE,		// don't create a depth buffer
				false);
			s_renderTarget = Pi::renderer->CreateRenderTarget(rtDesc);
		}

		static void SetRenderTargetCubemap(const Uint32 face, Graphics::Texture *pTexture)
		{
			s_renderTarget->SetCubeFaceTexture(face, pTexture);
		}

		static void BeginRenderTarget() {
			Pi::renderer->SetRenderTarget(s_renderTarget);
		}

		static void EndRenderTarget() {
			Pi::renderer->SetRenderTarget(nullptr);
		}

		static Graphics::RenderState* GetdRenderState() { return s_quadRenderState; }

		static Uint32 CPUTextureSize() { return m_textureSizeCpu; }
		static vector2f CPUTextureSize2f() { return vector2f(float(m_textureSizeCpu)); }

		static Uint32 GPUTextureSize() { return m_textureSizeGpu; }
		static vector2f GPUTextureSize2f() { return vector2f(float(m_textureSizeGpu)); }
	};
	GPUCloudSphereContext	*GPUCloudSphereContext::m_instance = nullptr;
	Graphics::RenderTarget	*GPUCloudSphereContext::s_renderTarget = nullptr;
	Graphics::RenderState	*GPUCloudSphereContext::s_quadRenderState = nullptr;
	Uint32 GPUCloudSphereContext::m_textureSizeCpu = 512;
	Uint32 GPUCloudSphereContext::m_textureSizeGpu = 1024;

	// ********************************************************************************
	// 
	// ********************************************************************************
	double fbm(const vector3d &position, const int octaves, float frequency, const float persistence) 
	{
		PROFILE_SCOPED()
		double total = 0.0;
		double maxAmplitude = 0.0;
		double amplitude = 1.0;
		for (int i = 0; i < octaves; i++) 
		{
			total += noise(position * frequency) * amplitude;
			frequency *= 2.0;
			maxAmplitude += amplitude;
			amplitude *= persistence;
		}
		return total / maxAmplitude;
	}

	
	double ridgedNoise(const vector3d &position, const int octaves, float frequency, const float persistence) 
	{
		double total = 0.0;
		double maxAmplitude = 0.0;
		double amplitude = 1.0;
		for (int i = 0; i < octaves; i++) {
			total += ((1.0 - abs(noise(position * frequency))) * 2.0 - 1.0) * amplitude;
			frequency *= 2.0;
			maxAmplitude += amplitude;
			amplitude *= persistence;
		}
		return total / maxAmplitude;
	}

	float CloudExpCurve(float v)
	{
		float c = std::max(v - CloudCover,0.0f);
		return 1.0 - pow(CloudSharpness, c);
	}
	

	// ********************************************************************************
	// 
	// ********************************************************************************
	CloudCPUGenRequest::CloudCPUGenRequest(const SystemPath &sysPath_, const Sint32 face_, bool (*callback)(const SystemPath &, CloudCPUGenResult *)) :
		corners(&s_patchFaces[Clamp(face_,0,NUM_PATCHES)][0]), sysPath(sysPath_), face(face_), m_callback(callback)
	{
		GPUCloudSphereContext *pt = GPUCloudSphereContext::Instance();
		const Uint32 xyDims = GPUCloudSphereContext::CPUTextureSize();
		colors = new Color[xyDims*xyDims];
	}

	// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	// Use only data local to this object
	void CloudCPUGenRequest::OnRun()
	{
		PROFILE_SCOPED()
		//MsgTimer timey;
		const Uint32 xyDims = GPUCloudSphereContext::CPUTextureSize();

		assert( corners != nullptr );
		const double fracStep = 1.0 / double(xyDims-1);
		for( Uint32 v=0; v<xyDims; v++ ) {
			for( Uint32 u=0; u<xyDims; u++ ) {
				// where in this row & colum are we now.
				const double ustep = double(u) * fracStep;
				const double vstep = double(v) * fracStep;

				// get point on the surface of the sphere
				const vector3d v_texCoord3D = GetSpherePoint(ustep, vstep);

				// generate some noise clouds
				// calculate solar heating + height & water contributions
				float distort = fbm(v_texCoord3D, 5, 2, 0.5) * 0.25;
				float intensity = Clamp((1.0 - abs(v_texCoord3D.y)) + distort, 0.0, 1.0);
				float heatAbsorption = intensity*1.5*ESun;
		
				// 1st cloud set
				float curvenoise = fbm(v_texCoord3D, 6, 8.0, 0.5) * 2.0;
				float curve = CloudExpCurve(curvenoise);
	
				// 2nd cloud set
				vector3d noisePosition = v_texCoord3D * 10.0;
				float noise = fbm(noisePosition, 6, 8.0, 0.5) * nScale;
				float rnoise = ridgedNoise(noisePosition, 4, 1.0, 0.5) * nScale;
				rnoise -= (1.0 - Density);
	
				// combine
				float thickness = std::max(rnoise * 2.0 + noise, 0.0);
				thickness = std::min(heatAbsorption * (((thickness * thickness)) + curve), 1.0f);
				vector3d texColor = vector3d(thickness,thickness,thickness)*2.0;
				// end of noise clouds

				// convert to ubyte and store
				Color* col = colors + (u + (v * xyDims));
				col[0].r = Uint8(texColor.x * 255.0);
				col[0].g = Uint8(texColor.y * 255.0);
				col[0].b = Uint8(texColor.z * 255.0);
				col[0].a = 255;
			}
		}

		//timey.Mark("SingleTextureFaceCPUJob::OnRun");
	}

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	CPUCloudSphereJob::~CPUCloudSphereJob()
	{
		PROFILE_SCOPED()
		if(mpResults) {
			mpResults->OnCancel();
			delete mpResults;
			mpResults = nullptr;
		}
	}

	void CPUCloudSphereJob::OnRun() // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	{
		PROFILE_SCOPED()
		mData->OnRun();

		// add this patches data
		CloudCPUGenResult *sr = new CloudCPUGenResult(mData->Face());
		sr->AddResult(mData->Colors(), GPUCloudSphereContext::CPUTextureSize());

		// store the result
		mpResults = sr;
	}

	void CPUCloudSphereJob::OnFinish() // runs in primary thread of the context
	{
		PROFILE_SCOPED()
		mData->InvokeCallback( mData->SysPath(), mpResults );
		mpResults = nullptr;
	}

	// ********************************************************************************
	GenFaceQuad::GenFaceQuad()
	{
		PROFILE_SCOPED()

		Graphics::MaterialDescriptor desc;
		desc.effect = Graphics::EFFECT_GEN_CLOUDSPHERE_TEXTURE;
		const Uint32 octaves = (Pi::config->Int("AMD_MESA_HACKS") == 0) ? 8 : 5;
		desc.quality = octaves;
		desc.textures = 2;
		m_material.reset(Pi::renderer->CreateMaterial(desc));

		// setup noise textures
		m_material->texture0 = Graphics::TextureBuilder::Raw("textures/permTexture.png").GetOrCreateTexture(Pi::renderer, "noise");
		m_material->texture1 = Graphics::TextureBuilder::Raw("textures/gradTexture.png").GetOrCreateTexture(Pi::renderer, "noise");
		
		// these might need to be reversed
		const vector2f &size(GPUCloudSphereContext::GPUTextureSize2f());

		Graphics::VertexArray vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

		vertices.Add(vector3f(0.0f,   0.0f,   0.0f), vector2f(0.0f,   size.y));
		vertices.Add(vector3f(0.0f,   size.y, 0.0f), vector2f(0.0f,   0.0f));
		vertices.Add(vector3f(size.x, 0.0f,   0.0f), vector2f(size.x, size.y));
		vertices.Add(vector3f(size.x, size.y, 0.0f), vector2f(size.x, 0.0f));

		//Create vtx & index buffers and copy data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_UV0;
		vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_FLOAT2;
		vbd.numVertices = vertices.GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		m_vertexBuffer.reset(Pi::renderer->CreateVertexBuffer(vbd));
		m_vertexBuffer->Populate(vertices);
	}

	void GenFaceQuad::Draw() {
		PROFILE_SCOPED()
		Pi::renderer->DrawBuffer(m_vertexBuffer.get(), GPUCloudSphereContext::GetdRenderState(), m_material.get(), Graphics::TRIANGLE_STRIP);
	}

	// ********************************************************************************
	CloudGPUGenRequest::CloudGPUGenRequest(const SystemPath &sysPath_, const float planetRadius_, bool (*callback)(const SystemPath &, CloudGPUGenResult *)) :
		sysPath(sysPath_), planetRadius(planetRadius_), m_callback(callback)
	{
		PROFILE_SCOPED()

		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector2f dataSize(GPUCloudSphereContext::GPUTextureSize2f());
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, 
			dataSize, texSize, Graphics::LINEAR_CLAMP, 
			true, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_texture.Reset(Pi::renderer->CreateTexture(texDesc));
	}
	
	void CloudGPUGenRequest::SetupMaterialParams(const int face)
	{
		PROFILE_SCOPED()
		m_specialParams.v = &s_patchFaces[face][0];
		m_specialParams.fracStep = 1.0f / float(GPUCloudSphereContext::GPUTextureSize());
		m_specialParams.planetRadius = planetRadius;
		m_specialParams.time = 0.0f;

		m_quad.GetMaterial()->specialParameter0 = &m_specialParams;
	}

	bool CloudGPUGenRequest::InvokeCallback(const SystemPath &sp, CloudGPUGenResult *cggr)
	{
		if(m_callback)
			return m_callback(sp, cggr);
		return false;
	}

	// ********************************************************************************
	void CloudGPUGenResult::AddResult(Graphics::Texture *t_, Sint32 uvDims_) {
		PROFILE_SCOPED()
		mData = CloudGPUGenData(t_, uvDims_);
	}

	void CloudGPUGenResult::OnCancel()	{
		if( mData.texture ) { mData.texture.Reset(); }
	}

	// ********************************************************************************
	// Overloaded JobGPU class to handle generating the mesh for each patch
	// ********************************************************************************
	GPUCloudSphereJob::GPUCloudSphereJob(CloudGPUGenRequest *data) : mData(data), mpResults(nullptr) 
	{ 
		/* empty */ 
		GPUCloudSphereContext::Instance();
	}

	GPUCloudSphereJob::~GPUCloudSphereJob()
	{
		PROFILE_SCOPED()
		if(mpResults) {
			mpResults->OnCancel();
			delete mpResults;
			mpResults = nullptr;
		}
	}

	void GPUCloudSphereJob::OnRun() // Runs in the main thread, may trash the GPU state
	{
		PROFILE_SCOPED()
		//MsgTimer timey;

		Pi::renderer->SetViewport(0, 0, GPUCloudSphereContext::GPUTextureSize(), GPUCloudSphereContext::GPUTextureSize());
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		// enter ortho
		{
			Pi::renderer->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
			Pi::renderer->PushMatrix();
			Pi::renderer->SetOrthographicProjection(0, GPUCloudSphereContext::GPUTextureSize(), GPUCloudSphereContext::GPUTextureSize(), 0, -1, 1);
			Pi::renderer->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
			Pi::renderer->PushMatrix();
			Pi::renderer->LoadIdentity();
		}

		GPUCloudSphereContext::BeginRenderTarget();
		for( Uint32 iFace=0; iFace<NUM_PATCHES; iFace++ )
		{
			// render the scene
			GPUCloudSphereContext::SetRenderTargetCubemap(iFace, mData->Texture());
			Pi::renderer->BeginFrame();

			// draw to the texture here
			mData->SetupMaterialParams( iFace );
			mData->Quad().Draw();

			Pi::renderer->EndFrame();
			GPUCloudSphereContext::SetRenderTargetCubemap(iFace, nullptr);
		}
		GPUCloudSphereContext::EndRenderTarget();

		// leave ortho?
		{
			Pi::renderer->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
			Pi::renderer->PopMatrix();
			Pi::renderer->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
			Pi::renderer->PopMatrix();
		}

		// add this patches data
		CloudGPUGenResult *sr = new CloudGPUGenResult();
		sr->AddResult( mData->Texture(), GPUCloudSphereContext::GPUTextureSize() );

		// store the result
		mpResults = sr;

		//timey.Mark("GPUCloudSphereJob::OnRun");
	}

	void GPUCloudSphereJob::OnFinish() // runs in primary thread of the context
	{
		PROFILE_SCOPED()
		mData->InvokeCallback(mData->SysPath(), mpResults);
		mpResults = nullptr;
	}
};

