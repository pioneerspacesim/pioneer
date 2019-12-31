// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"
#include "graphics/Light.h"

namespace Graphics {
	class RenderTarget;
}

// Forward declare this type from imgui.h
using ImTextureID = void *;

#include "Shields.h"
#include "scenegraph/Model.h"
#include "scenegraph/ModelSkin.h"

// TODO:
// - make this code reusable across multiple usecases (camera, rear-view mirror, UI preview, etc.)
// - Render more than a single model, e.g. attachments / equipment / shields (a scene, scenegraph hierarchy, etc.)
// - Add support for the usual post-processing chain (integrate with regular scene rendering)
// In essence, we want to make the regular main-scene rendering code modular and able to render
// arbitrary scenes to RTs that we can use in Pigui et. al.
// For now we'll do a basic implementation here to move the model spinner functionality to Pigui.
namespace PiGUI {
	class ModelSpinner : public RefCounted {
	public:
		ModelSpinner();

		// Set the ship we should be looking at.
		void SetModel(SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, unsigned int pattern);

		// Called to draw the model to the render target.
		void Render();

		// Draws the model spinner widget and handles user interaction.
		// Expected to be called during a Begin/End ImGui block.
		// The modelspinner attempts to take all the available horizontal space
		// in the window - use ImGui::BeginChild() to constrain the sizing.
		void DrawPiGui();

		// Set the size of the texture to render.
		void SetSize(vector2d size);

		// Retrieve the size of the rendered texture.
		const vector2d GetSize() { return static_cast<vector2d>(m_size); }

		// Transform a model-space location into a screen-space position.
		vector2d ModelSpaceToScreenSpace(vector3d modelSpaceVec);

	private:
		std::unique_ptr<Graphics::RenderTarget> m_renderTarget;
		std::unique_ptr<SceneGraph::Model> m_model;
		SceneGraph::ModelSkin m_skin;
		std::unique_ptr<Shields> m_shields;
		Graphics::Light m_light;

		void CreateRenderTarget();
		ImTextureID GetTextureID();

		// The size of the render target.
		vector2f m_size;
		// Do we need to resize the render target next frame?
		bool m_needsResize;

		// After the user manually rotates the model, hold that orientation for
		// a second to let them look at it. Assumes Update() is called every
		// frame while visible.
		float m_pauseTime;

		// The rotation of the model.
		vector2f m_rot;
	};
} // namespace PiGUI
