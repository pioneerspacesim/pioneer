// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "render/SceneRenderer.h"

namespace Render {

	class FullscreenGenerator : public RenderGenerator {
	public:
		using RenderGenerator::RenderGenerator;

		void Run(RenderPass *pass, Space *space, const CameraContext *camera) override;
	};

	class FullscreenResolveGenerator : public RenderGenerator {
	public:
		using RenderGenerator::RenderGenerator;

		void Run(RenderPass *pass, Space *space, const CameraContext *camera) override;
	};

	class FullscreenDownsampleGenerator : public RenderGenerator {
	public:
		using RenderGenerator::RenderGenerator;

		void CacheShadersForPass(RenderPass *pass) override;
		void Run(RenderPass *pass, Space *space, const CameraContext *camera) override;
	};

}
