// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"
#include "imgui/imgui.h"
#include "vector2.h"

namespace PiGui {
	class RadarWidget : public RefCounted {
	public:
		// Draws the radar widget
		// Expected to be called during a Begin/End ImGui block.
		void DrawPiGui();

		// Set the total size of the radar widget
		void SetSize(ImVec2 size);
		// Return the total size of the radar widget
		ImVec2 GetSize() const { return m_size; }

		// Set the current zoom distance in meters
		void SetCurrentZoom(float zoomDist) { m_currentZoom = zoomDist; }
		// Return the current zoom distance in meters
		float GetCurrentZoom() const { return m_currentZoom; }

		// Set the maximum zoom distance in meters
		void SetMaxZoom(float maxZoom) { m_maxZoom = maxZoom; }
		// Return the maximum zoom distance in meters
		float GetMaxZoom() const { return m_maxZoom; }

		// Set the minimum zoom distance in meters
		void SetMinZoom(float minZoom) { m_minZoom = minZoom; }
		// Return the minimum zoom distance in meters
		float GetMinZoom() const { return m_minZoom; }

		// Retrieve the size of the radar disk itself
		ImVec2 GetRadius() const { return m_radius; }
		// Return the position of the center of the radar disk
		ImVec2 GetCenter() const { return m_center; }

	private:
		ImVec2 m_size;
		ImVec2 m_radius;
		ImVec2 m_center;

		float m_currentZoom;
		float m_maxZoom;
		float m_minZoom;
	};
} // namespace PiGui
