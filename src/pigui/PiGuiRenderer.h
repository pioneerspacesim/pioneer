// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Renderer.h"
#include <memory>

struct ImDrawData;

namespace PiGui {

	class InstanceRenderer {
	public:
		InstanceRenderer(Graphics::Renderer *r);

		void Initialize();
		void Shutdown();

		void NewFrame() {}
		void RenderDrawData(ImDrawData *draw_data);

		void CreateFontsTexture();
		void DestroyFontsTexture();

	private:
		Graphics::Renderer *m_renderer;

		std::unique_ptr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::VertexBuffer> m_vtxBuffer;
		std::unique_ptr<Graphics::IndexBuffer> m_idxBuffer;
		std::unique_ptr<Graphics::Texture> m_fontsTexture;
	};
} // namespace PiGui
