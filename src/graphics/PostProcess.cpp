// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PostProcess.h"
#include "Renderer.h"
#include "WindowSDL.h"
#include "Material.h"

namespace Graphics {

PostProcess::PostProcess(const std::string& effect_name, RenderTargetDesc& rtd) :
	strName(effect_name)
{
	mRTDesc.reset(new RenderTargetDesc(
		rtd.width, rtd.height, rtd.colorFormat, rtd.depthFormat, rtd.allowDepthTexture));
}

PostProcess::PostProcess(const std::string& effect_name, WindowSDL* window) :
	strName(effect_name)
{
	assert(window != nullptr);
	mRTDesc.reset(new RenderTargetDesc(
		window->GetWidth(), window->GetHeight(),
		TextureFormat::TEXTURE_RGB_888, TextureFormat::TEXTURE_NONE,
		false));
}

PostProcess::~PostProcess()
{
	for(unsigned int i = 0; i < vPasses.size(); ++i) {
		delete vPasses[i];
	}
}

void PostProcess::AddPass(Renderer* renderer, const std::string& pass_name, 
	std::shared_ptr<Material>& material, PostProcessPassType pass_type)
{
	assert(renderer != nullptr);
	PostProcessPass* ppp = new PostProcessPass;
	ppp->name = pass_name;
	ppp->material = material;
	ppp->type = pass_type;
	if(vPasses.size() > 0) { // Pass creates rendertarget for pass before it, last pass never requires a render target
		vPasses.back()->renderTarget.reset(renderer->CreateRenderTarget(*mRTDesc.get()));
	}
	vPasses.push_back(ppp);
}

void PostProcess::AddPass(Renderer* renderer, const std::string& pass_name, Graphics::EffectType effect_type, PostProcessPassType pass_type)
{
	assert(renderer != nullptr);
	MaterialDescriptor md;
	md.effect = effect_type;
	std::shared_ptr<Material> mtrl;
	mtrl.reset(renderer->CreateMaterial(md));
	return AddPass(renderer, pass_name, mtrl, pass_type);
}

}