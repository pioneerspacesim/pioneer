// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "render/SceneRenderer.h"

class Body;

namespace Render {

	class LightingGenerator : public RenderGenerator {
	public:
		using RenderGenerator::RenderGenerator;

		void Run(RenderPass *pass, Space *space, const CameraContext *camera) override;
	};

}
