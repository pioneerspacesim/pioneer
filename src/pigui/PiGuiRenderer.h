// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

		// Render a draw data to the screen using an orthographic projection
		void RenderDrawData(ImDrawData *draw_data);

		// Render a draw data struct using a user-defined material and rendering setup.
		// The user is responsible for setting the transform and projection matrices before drawing.
		// The provided material should support two parameters:
		//  - texture0:     Texture2d
		//  - vertexDepth:  float
		void RenderDrawData(ImDrawData *draw_data, Graphics::Material* material);

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
