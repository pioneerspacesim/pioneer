// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Texture.h"

#include <memory>
#include <string_view>
#include <map>
#include <vector>

namespace Graphics {
	class Material;
	class Renderer;
	class RenderTarget;
	struct RenderTargetDesc;
}

namespace Render {

	struct RenderTargetResource;

	class RenderPass;
	class RenderSetup;

	/**
	 * ResourceManager holds all per-frame and long-term resources used by a
	 * high-level rendering pipeline as defined in the associated RenderSetup.
	 *
	 * It serves as a resource allocation API for individual RenderGenerators
	 * to request per-frame transient resources (i.e. intermediate targets),
	 * and manages defining render resources in terms of existing resources
	 * (e.g. half-res or quarter-res targets).
	 *
	 * Currently the ResourceManager only stores render targets, but it could
	 * easily be extended to support statically-referenced textures needed as
	 * inputs for various post-processing shaders, etc.
	 *
	 * TODO: add mechanism to recreate resources which depend on the window
	 *       backbuffer when the backbuffer size or format (e.g. MSAA) changes.
	 */
	class ResourceManager {
	public:
		ResourceManager(Graphics::Renderer *r);
		~ResourceManager();

		void LoadRenderSetup(RenderSetup *rs);

		void BeginFrame();

		void ImportRenderTarget(Graphics::RenderTarget *target, std::string_view name);
		bool CreateRenderTarget(const RenderTargetResource &res);

		Graphics::RenderTarget *GetRenderTarget(std::string_view name);

		Graphics::RenderTarget *GetTempRenderTarget(uint32_t width, uint32_t height, Graphics::TextureFormat format, bool msaa);
		void ReleaseTempRenderTarget(Graphics::RenderTarget *);

	private:
		enum TempState : uint8_t {
			STATE_IN_USE = 1 << 0,
			STATE_USED_THIS_FRAME = 1 << 1
		};

		struct TempRenderTarget {
			TempRenderTarget(uint8_t u, Graphics::RenderTarget *r) :
				used(u),
				rt(r)
			{}

			uint8_t used;
			std::unique_ptr<Graphics::RenderTarget> rt;
		};

		Graphics::RenderTargetDesc GetRTDescForDependent(std::string_view name);

		Graphics::Renderer *m_renderer;
		uint32_t m_numMsaaSamples;

		std::map<std::string, Graphics::RenderTarget *, std::less<>> m_renderTargets;
		std::vector<std::unique_ptr<Graphics::RenderTarget>> m_ownedRenderTargets;
		std::vector<TempRenderTarget> m_tempRenderTargets;
	};

} // namespace Render
