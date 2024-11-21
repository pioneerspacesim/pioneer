// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ResourceManager.h"

#include "core/Log.h"
#include "graphics/Renderer.h"
#include "graphics/RenderTarget.h"
#include "graphics/Texture.h"
#include "render/RenderSetup.h"

#include "profiler/Profiler.h"

using namespace Render;

ResourceManager::ResourceManager(Graphics::Renderer *r) :
	m_renderer(r)
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::LoadRenderSetup(RenderSetup *rs)
{
	PROFILE_SCOPED()

	// Setup default msaa sample number for render targets
	m_numMsaaSamples = rs->Properties()->Get("msaa").get_integer(0);

	const std::vector<RenderResource> &global_res = rs->GetGlobalResources();

	for (const RenderResource &res : global_res) {

		// TODO: support other RenderResource types once implemented
		if (const auto *rt = std::get_if<RenderTargetResource>(&res)) {
			CreateRenderTarget(*rt);
		}

	}
}

void ResourceManager::BeginFrame()
{
	PROFILE_SCOPED()

	// Release all temporary render targets which were not used at all the prior frame
	// NOTE: render targets can be used -> released -> reused on the same or immediately
	// following frames to avoid VRAM allocation fragmentation for consistently used RTs
	for (size_t i = m_tempRenderTargets.size(); i > 0; i--) {
		auto &temp = m_tempRenderTargets[i - 1];

		// Not in use or used the previous frame, clean it up to save VRAM
		if (!temp.used) {
			temp.rt.reset();
			m_tempRenderTargets.erase(m_tempRenderTargets.cbegin() + i);
		}
	}

	// Unmark render targets as in-use
	for (auto &temp : m_tempRenderTargets) {
		temp.used &= ~STATE_USED_THIS_FRAME;
	}
}

bool ResourceManager::CreateRenderTarget(const RenderTargetResource &res)
{
	PROFILE_SCOPED()

	Graphics::RenderTargetDesc depends_on = GetRTDescForDependent(res.depends_on);

	uint16_t width = float(depends_on.width) * res.width_scale;
	uint16_t height = float(depends_on.height) * res.height_scale;

	Graphics::RenderTargetDesc rtdesc = {
		width, height, res.format, res.depthFormat,
		res.depthFormat != Graphics::TEXTURE_NONE,
		uint16_t(res.multisampled ? m_numMsaaSamples : 0)
	};

	Graphics::RenderTarget *rt = m_renderer->CreateRenderTarget(rtdesc);

	if (!rt) {
		Log::Error("Render::ResourceManager: Cannot create global render target '{}' with size {}x{}, format {} (depth {}), {} samples",
			res.name, width, height, int(res.format), int(res.depthFormat), res.multisampled ? m_numMsaaSamples : 0);
		return false;
	}

	m_ownedRenderTargets.emplace_back(rt);
	m_renderTargets[res.name] = rt;
	return true;
}

void ResourceManager::ImportRenderTarget(Graphics::RenderTarget *rt, std::string_view name)
{
	m_renderTargets[std::string(name)] = rt;
}

Graphics::RenderTarget *ResourceManager::GetRenderTarget(std::string_view name)
{
	auto iter = m_renderTargets.find(name);
	if (iter != m_renderTargets.end())
		return iter->second;

	return nullptr;
}

Graphics::RenderTarget *ResourceManager::GetTempRenderTarget(uint32_t width, uint32_t height, Graphics::TextureFormat format, bool msaa)
{
	PROFILE_SCOPED()

	uint32_t numSamples = (msaa ? m_numMsaaSamples : 0);

	// Attempt to find a compatible unused render target in our list of temporary RTs
	for (auto &temp : m_tempRenderTargets) {
		if (temp.used & STATE_IN_USE)
			continue;

		const Graphics::RenderTargetDesc &desc = temp.rt->GetDesc();
		if (desc.width == width && desc.height == height && desc.colorFormat == format && desc.numSamples == numSamples) {
			temp.used = STATE_IN_USE | STATE_USED_THIS_FRAME;
			return temp.rt.get();
		}
	}

	// Can't find it, we'll make one.
	Graphics::RenderTargetDesc desc = {
		uint16_t(width), uint16_t(height),
		format, Graphics::TEXTURE_NONE,
		false, uint16_t(numSamples)
	};

	Graphics::RenderTarget *rt = m_renderer->CreateRenderTarget(desc);

	if (!rt) {
		Log::Error("Render::ResourceManager: Cannot create temporary render target with size {}x{}, format {}, {} samples",
			width, height, int(format), numSamples);
	}

	m_tempRenderTargets.emplace_back(uint8_t(STATE_IN_USE | STATE_USED_THIS_FRAME), rt);
	return rt;
}

void ResourceManager::ReleaseTempRenderTarget(Graphics::RenderTarget *target)
{
	// Clear the in-use state bit on this render target (can be re-used this frame)
	for (auto &temp : m_tempRenderTargets) {
		if (temp.rt.get() != target)
			continue;

		temp.used &= ~STATE_IN_USE;
		return;
	}

	assert(false && "Render::ResourceManager cannot release a temporary render target it didn't create.");
}

Graphics::RenderTargetDesc ResourceManager::GetRTDescForDependent(std::string_view name)
{
	// Look up a render target descriptor for a render target, with special handling for the window backbuffer
	if (auto iter = m_renderTargets.find(name); iter != m_renderTargets.end()) {
		return iter->second->GetDesc();
	} else if (name.compare("window") == 0) {
		return Graphics::RenderTargetDesc {
			uint16_t(m_renderer->GetWindowWidth()),
			uint16_t(m_renderer->GetWindowHeight()),
			Graphics::TEXTURE_NONE,
			Graphics::TEXTURE_NONE,
			false, 0
		};
	} else {
		return Graphics::RenderTargetDesc { 1, 1, Graphics::TEXTURE_NONE, Graphics::TEXTURE_NONE, false, 0 };
	}
}
