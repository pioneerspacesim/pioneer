
#include "Radar.h"
#include "MathUtil.h"
#include "imgui/imgui.h"
#include "profiler/Profiler.h"

using RadarWidget = PiGui::RadarWidget;

static constexpr int RADAR_STEPS = 100;

ImVec2 circlePos(float a, ImVec2 center, ImVec2 radius, float scale = 1.0f)
{
	return ImVec2(center.x + sin(a) * scale * radius.x, center.y + cos(a) * scale * radius.y);
}

void RadarWidget::SetSize(ImVec2 size)
{
	m_size = size;
	m_radius = ImVec2(m_size.x / 2.f - 2.f, m_size.y / 3.f - 2.f);
}

void RadarWidget::DrawPiGui()
{
	PROFILE_SCOPED()

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_center = ImVec2(pos.x + m_size.x / 2.f, pos.y + m_size.y / 2.f);
	ImVec2 zoomPos = ImVec2(m_center.x, m_center.y + (m_size.y / 2.6f) - m_radius.y);

	static const float circle = float(2 * M_PI);
	static const float step = circle / RADAR_STEPS;

	// circle
	for (float ang = 0; ang < circle; ang += step) {
		drawList->PathLineTo(circlePos(ang, m_center, m_radius));
	}
	drawList->PathFillConvex(ImGui::GetColorU32(ImGuiCol_FrameBg));

#if 0
	float cAlpha = std::max(sin(float(SDL_GetTicks()) / 500.0f), 0.f);
	float scanSize = MathUtil::Lerp(0.1f, 1.0f, cAlpha);
	// scan ring
	for (float ang = 0; ang < circle; ang += step) {
		drawList->PathLineTo(circlePos(ang, m_center, m_radius, scanSize));
	}

	ImVec4 brightCol = ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered];
	brightCol.w = MathUtil::Lerp(0.1, brightCol.w * 0.6, cAlpha);
	drawList->PathFillConvex(ImColor(brightCol));
#endif

	// dynamic lines
	for (int i = 0; i < 16; i++) {
		float dist = (powf(2.0, float(i)) * 1000.f) / m_currentZoom;
		if (dist < 0.1f) continue;
		if (dist > 1.0f) break;

		for (float ang = 0; ang < circle; ang += step) {
			drawList->PathLineTo(circlePos(ang, m_center, m_radius, dist));
		}
		drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBgActive), true);
	}

	// outer ring
	for (float ang = 0; ang < circle; ang += step) {
		drawList->PathLineTo(circlePos(ang, m_center, m_radius));
	}
	drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBgActive), true, 2.0);

	// inner ring
	for (float ang = 0; ang < circle; ang += circle / 20.f) {
		drawList->PathLineTo(circlePos(ang, m_center, m_radius, 0.1f));
	}
	drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBgActive), true);

	// spokes
	for (float ang = 0; ang < circle; ang += circle / 8.f) {
		drawList->PathLineTo(circlePos(ang, m_center, m_radius, 0.1f));
		drawList->PathLineTo(circlePos(ang, m_center, m_radius));
		drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBgActive), false);
	}

	// total zoom bar
	const float zoomArc = float(0.5 * M_PI);
	for (float ang = 0; ang < zoomArc; ang += step) {
		drawList->PathLineTo(circlePos(ang - zoomArc / 2.f, zoomPos, m_radius));
	}
	drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 6.0f);

	// current zoom bar
	const float zoomRingPct = zoomArc * (log10(m_currentZoom / m_minZoom) / log10(m_maxZoom / m_minZoom));
	for (float ang = 0; ang < zoomRingPct; ang += step / 8.f) {
		drawList->PathLineTo(circlePos(ang - zoomArc / 2.f, zoomPos, m_radius));
	}
	drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBgActive), false, 6.0f);
}
