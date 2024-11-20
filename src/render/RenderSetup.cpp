// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RenderSetup.h"

#include "FileSystem.h"
#include "core/Log.h"

#include "core/StringUtils.h"
#include "graphics/Texture.h"
#include "toml11/toml.hpp"

using namespace Render;

static Graphics::TextureFormat parse_format(std::string_view format)
{
	if (compare_ci(format, "RGBA8")) {
		return Graphics::TEXTURE_RGBA_8888;
	} else if (compare_ci(format, "RGB8")) {
		return Graphics::TEXTURE_RGB_888;
	} else if (compare_ci(format, "RG8")) {
		return Graphics::TEXTURE_RG_88;
	} else if (compare_ci(format, "R8")) {
		return Graphics::TEXTURE_R8;
	} else if (compare_ci(format, "D32F")) {
		return Graphics::TEXTURE_DEPTH;
	} else {
		return Graphics::TEXTURE_NONE;
	}
}

RenderSetup::RenderSetup(std::string_view renderSetupPath)
{
	auto fileData = FileSystem::gameDataFiles.ReadFile(std::string(renderSetupPath));

	auto result = toml::try_parse_str(std::string(fileData->AsStringView()));

	if (result.is_err()) {
		for (auto &err : result.as_err()) {
			Log::Error("Error reading render setup config {}: {}", renderSetupPath, err.title());
		}
	}

	const toml::value &setup = result.as_ok();

	m_properties.reset(new PropertyMap());

	// Initialize default renderer properties

	for (const auto &param : toml::find_or(setup, "parameters", toml::table())) {
		if (param.second.is_string()) {
			m_properties->Set(param.first, param.second.as_string());
		} else if (param.second.is_integer()) {
			m_properties->Set(param.first, param.second.as_integer());
		} else if (param.second.is_floating()) {
			m_properties->Set(param.first, param.second.as_floating());
		} else if (param.second.is_boolean()) {
			m_properties->Set(param.first, param.second.as_boolean());
		}
	}

	// Create global resource lists

	for (const auto &rt : toml::find_or(setup, "resource", "render_target", toml::array())) {

		RenderTargetResource res = {};

		res.name = toml::find_or(rt, "name", "");

		if (res.name.empty()) {
			Log::Error("Empty name for render target resource in render setup {}", renderSetupPath);
			continue;
		}

		std::string format = toml::find_or(rt, "format", "");
		res.format = parse_format(format);

		if (res.format == Graphics::TEXTURE_NONE) {
			Log::Error("Invalid color format '{}' for render target {} in render setup {}", format, res.name, renderSetupPath);
			continue;
		}

		std::string depth = toml::find_or(rt, "depth", "");
		res.depthFormat = parse_format(depth);

		res.depends_on = toml::find_or(rt, "depends_on", "");

		res.height_scale = toml::find_or(rt, "h_scale", 1.0f);
		res.width_scale = toml::find_or(rt, "w_scale", 1.0f);

		res.multisampled = toml::find_or(rt, "multisample", false);

		m_globalResources.push_back(std::move(res));

	}

	// Build render layer configs

	for (const auto &[name, layer] : toml::find_or(setup, "render_pass", toml::table())) {

		m_renderLayers.push_back(name);
		std::vector<RenderPass> &passes = m_renderLayerConfigs[name];

		for (const auto &def : layer.as_array()) {

			RenderPass pass = {};

			pass.name = toml::find_or(def, "name", "");

			if (pass.name.empty()) {
				Log::Error("Empty name for render layer '{}' render pass in render setup {}", name, renderSetupPath);
				continue;
			}

			pass.render_target = toml::find_or(def, "render_target", "");

			pass.input = toml::find_or(def, "input", "");

			pass.generator = toml::find_or(def, "generator", "");

			pass.shader = toml::find_or(def, "shader", "");

			pass.steps = toml::find_or(def, "steps", 0);

			// TODO: conditions

			passes.push_back(std::move(pass));

		}

	}

}

RenderSetup::~RenderSetup()
{
}

std::vector<RenderPass *> RenderSetup::GetRenderLayerConfig(std::string_view layerName)
{
	auto iter = m_renderLayerConfigs.find(layerName);

	if (iter == m_renderLayerConfigs.end())
		return {};

	std::vector<RenderPass *> outRenderPasses;

	for (auto &pass : iter->second) {
		if (EvaluateConditions(&pass)) {
			outRenderPasses.push_back(&pass);
		}
	}

	return outRenderPasses;
}

const std::vector<RenderPass> &RenderSetup::GetAllRenderPassesInLayer(std::string_view layerName) const
{
	return m_renderLayerConfigs.at(std::string(layerName));
}

bool RenderSetup::EvaluateConditions(RenderPass *pass)
{
	return true; // FIXME: evaluate each individual condition block in the render pass
}
