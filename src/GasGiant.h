// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASGIANT_H
#define _GASGIANT_H

#include "BaseSphere.h"
#include "GasGiantJobs.h"
#include "JobQueue.h"
#include "vector3.h"

#include <deque>

namespace Graphics {
	class Renderer;
	class RenderTarget;
	class Texture;
} // namespace Graphics

class SystemBody;
class GasGiant;
class GasPatch;
class GasPatchContext;
class Camera;

namespace {
	class STextureFaceResult;
	class SGPUGenResult;
} // namespace

#define NUM_PATCHES 6

class GasGiant : public BaseSphere {
public:
	GasGiant(const SystemBody *body);
	virtual ~GasGiant();

	virtual void Update() override;
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows) override;

	virtual double GetHeight(const vector3d &p) const override final { return 0.0; }

	// in sbody radii
	virtual double GetMaxFeatureHeight() const override { return 0.0; }

	virtual void Reset() override;

	static bool OnAddTextureFaceResult(const SystemPath &path, GasGiantJobs::STextureFaceResult *res);
	static bool OnAddGPUGenResult(const SystemPath &path, GasGiantJobs::SGPUGenResult *res);
	static void Init();
	static void Uninit();
	static void UpdateAllGasGiants();
	static void OnChangeDetailLevel();

	static void CreateRenderTarget(const Uint16 width, const Uint16 height);
	static void SetRenderTargetCubemap(const Uint32, Graphics::Texture *, const bool unBind = true);
	static Graphics::RenderTarget *GetRenderTarget();
	static void BeginRenderTarget();
	static void EndRenderTarget();

private:
	void BuildFirstPatches();
	void GenerateTexture();
	bool AddTextureFaceResult(GasGiantJobs::STextureFaceResult *res);
	bool AddGPUGenResult(GasGiantJobs::SGPUGenResult *res);

	static RefCountedPtr<GasPatchContext> s_patchContext;

	static Graphics::RenderTarget *s_renderTarget;

	std::unique_ptr<GasPatch> m_patches[NUM_PATCHES];

	bool m_hasTempCampos;
	vector3d m_tempCampos;

	virtual void SetUpMaterials() override;
	RefCountedPtr<Graphics::Texture> m_surfaceTextureSmall;
	RefCountedPtr<Graphics::Texture> m_surfaceTexture;
	RefCountedPtr<Graphics::Texture> m_builtTexture;

	std::unique_ptr<Color[]> m_jobColorBuffers[NUM_PATCHES];
	Job::Handle m_job[NUM_PATCHES];
	bool m_hasJobRequest[NUM_PATCHES];

	Job::Handle m_gpuJob;
	bool m_hasGpuJobRequest;

	float m_timeDelay;
};

#endif /* _GASGIANT_H */
