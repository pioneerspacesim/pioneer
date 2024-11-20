// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "core/Property.h"
#include "graphics/Texture.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace Render {

	/**
	 * RenderPass stores information about a specific render pass, including
	 * generator name, render targets, (TODO) input parameters to invoked
	 * shaders, and (TODO) a list of conditions which must be met to execute
	 * this render pass.
	 */
	struct RenderPass {
		std::string name;
		std::string render_target; // TODO: split this into render_targets[] and depth_target
		std::string generator;
		std::string shader;
		std::string input;
		uint32_t steps;
	};

	/**
	 * A RenderTargetResource stores information about a render target texture
	 * the render_setup file needs to implement its rendering.
	 */
	struct RenderTargetResource {
		std::string name;
		std::string depends_on;
		Graphics::TextureFormat format;
		Graphics::TextureFormat depthFormat;
		float width_scale = 1;
		float height_scale = 1;
		bool multisampled = false;
	};

	using RenderResource = std::variant<RenderTargetResource>;

	/**
	 * Responsible for loading and managing a render configuration defined in
	 * a render_setup file.
	 *
	 * The RenderSetup owns the PropertyMap used to control which RenderPasses
	 * are activated and serves as the datasource for passing parameters to
	 * individual shaders as defined (TODO) in a RenderPass.
	 */
	class RenderSetup {
	public:
		RenderSetup(std::string_view renderSetupPath);
		~RenderSetup();

		// Return a list of all global resources defined by this render setup file
		const std::vector<RenderResource> &GetGlobalResources() const { return m_globalResources; }

		// Return a list of all render layers specified by this render setup file
		const std::vector<std::string> &GetRenderLayers() const { return m_renderLayers; }

		// Return the list of all defined render passes in the specified render layer.
		// NOTE: some of these passes may not be used at draw time depending on the configuration of the render setup
		const std::vector<RenderPass> &GetAllRenderPassesInLayer(std::string_view layerName) const;

		// Return a list of active render passes which make up the specified
		// layer, based on the current value of the contained property map.
		// NOTE: this function evaluates all conditions associated with the
		// contained render passes and should not be treated as a fast accessor.
		std::vector<RenderPass *> GetRenderLayerConfig(std::string_view layerName);

		PropertyMap *Properties() { return m_properties.get(); }

	private:
		bool EvaluateConditions(RenderPass *pass);

		std::unique_ptr<PropertyMap> m_properties;
		std::vector<RenderResource> m_globalResources;
		std::vector<std::string> m_renderLayers;
		std::map<std::string, std::vector<RenderPass>, std::less<>> m_renderLayerConfigs;
	};

} // namespace Render
