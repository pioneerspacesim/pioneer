// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "ViewportWindow.h"

#include "Color.h"
#include "Input.h"
#include "core/Log.h"

#include "vector2.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "matrix4x4.h"

#include <string_view>

class NavLights;

namespace SceneGraph {
	class Model;
	class Animation;
}

namespace Graphics {
	class MeshObject;
	class Material;

	namespace Drawables {
		class GridLines;
	}
}

namespace Editor
{
	using UIDelegate = sigc::signal<void>;

	class ModelViewerWidget : public ViewportWindow {
	public:
		struct Options {
			bool orthoView;
			bool mouselookEnabled;
			bool hideUI;

			bool showGrid;
			bool showVerticalGrids;
			float gridInterval;

			bool showAabb;
			bool showCollMesh;
			bool showTags;
			bool showDockingLocators;
			bool showGeomBBox;
			bool wireframe;

			bool showLandingPad;

			uint32_t lightPreset;
		};

		enum class CameraPreset : uint8_t {
			Front,
			Back,
			Left,
			Right,
			Top,
			Bottom
		};

	public:
		ModelViewerWidget(EditorApp *app);
		~ModelViewerWidget();

		bool LoadModel(std::string_view path);
		void ClearModel();

		void OnAppearing() override;
		void OnDisappearing() override;

		void SetDecals(std::string_view decalPath);
		void SetRandomColors();

		const char *GetWindowName() override;

		Options &GetOptions() { return m_options; }

		SceneGraph::Model *GetModel();

		const matrix4x4f &GetModelViewMat() const { return m_modelViewMat; }

		// Extend to render in-world using Graphics::Renderer
		UIDelegate &GetUIExtPostRender() { return m_extPostRender; }
		// Extend to render on top of the viewport surface using ImDrawList
		UIDelegate &GetUIExtOverlay() { return m_extOverlay; }
		// Extend to add additional viewport menu buttons
		UIDelegate &GetUIExtMenu() { return m_extMenus; }
		// Extend to add additional viewport control widgets
		UIDelegate &GetUIExtViewportControls() { return m_extViewportControls; }

	protected:

		void OnUpdate(float deltaTime) override;
		void OnRender(Graphics::Renderer *r) override;
		void OnHandleInput(bool clicked, bool released, ImVec2 mousePos) override;
		void OnDraw() override;

		bool OnCloseRequested() override { return true; };

		UIDelegate m_extPostRender;
		UIDelegate m_extOverlay;
		UIDelegate m_extMenus;
		UIDelegate m_extViewportControls;

	private:
		struct Inputs : Input::InputFrame {
			using InputFrame::InputFrame;

			Axis *moveForward;
			Axis *moveLeft;
			Axis *moveUp;
			Axis *zoomAxis;

			Axis *rotateViewLeft;
			Axis *rotateViewUp;

			Action *viewTop;
			Action *viewLeft;
			Action *viewFront;
		} m_bindings;

		// Initialization
		void SetupInputAxes();
		void CreateTestResources();
		void OnModelLoaded();

		// Input controls
		void ChangeCameraPreset(CameraPreset preset);
		void ToggleViewControlMode();
		void ResetCamera();
		void HandleCameraInput(float deltaTime);

		// Interface drawing
		void DrawMenus();
		void DrawViewportControls();

		// Viewport rendering
		void UpdateCamera();
		void UpdateLights();
		void DrawBackground();
		void DrawModel(matrix4x4f modelViewMat);
		void DrawGrid(Graphics::Renderer *r, float clipRadius);

		Input::Manager *m_input;
		Graphics::Renderer *m_renderer;

		std::unique_ptr<SceneGraph::Model> m_model;
		std::unique_ptr<NavLights> m_navLights;

		std::unique_ptr<Graphics::MeshObject> m_bgMesh;
		std::unique_ptr<Graphics::Material> m_bgMaterial;

		std::unique_ptr<Graphics::Drawables::GridLines> m_gridLines;
		std::unique_ptr<SceneGraph::Model> m_scaleModel;

		std::vector<SceneGraph::Animation *> m_animations;
		SceneGraph::Animation *m_currentAnimation = nullptr;

		bool m_modelSupportsPatterns = false;
		std::vector<std::string> m_patterns;
		uint32_t m_currentPattern = 0;

		Graphics::Texture *m_decalTexture;

		Options m_options;

		// Model pattern colors
		std::vector<Color> m_colors;

		std::string m_windowName;
		std::string m_windowID;

		float m_baseDistance;
		float m_gridDistance;
		float m_landingMinOffset;
		float m_zoom;
		vector2f m_rot;
		vector3f m_viewPos;
		matrix3x3f m_viewRot;
		matrix4x4f m_modelViewMat;
	};
}
