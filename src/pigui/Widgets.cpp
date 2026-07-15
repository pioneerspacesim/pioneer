// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Input.h"
#include "Pi.h"

#include "Widgets.h"

// For ImRect
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <profiler/Profiler.h>

using namespace PiGui;

int Draw::RadialPopupSelectMenu(const ImVec2 center, const char *popup_id, int mouse_button, const std::vector<ImTextureID> &tex_ids, const std::vector<std::pair<ImVec2, ImVec2>> &uvs, const std::vector<ImU32> &colors, const std::vector<const char *> &tooltips, unsigned int size, unsigned int padding)
{
	PROFILE_SCOPED()
	// return:
	// 0 - nothing is selected
	// > 0 - item selected
	int ret = 0;

	static InputBindings::Axis *horizontalSelection = Pi::input->GetAxisBinding("BindRadialHorizontalSelection");
	static InputBindings::Axis *verticalSelection = Pi::input->GetAxisBinding("BindRadialVerticalSelection");

	ImColor bgCol = ImGui::GetColorU32(ImGuiCol_PopupBg);
	ImColor itemBgCol = ImGui::GetColorU32(ImGuiCol_Button);
	ImColor itemHoveredCol = ImGui::GetColorU32(ImGuiCol_ButtonHovered);

	// FIXME: Missing a call to query if Popup is open so we can move the PushStyleColor inside the BeginPopupBlock (e.g. IsPopupOpen() in imgui.cpp)
	// FIXME: Our PathFill function only handle convex polygons, so we can't have items spanning an arc too large else inner concave edge artifact is too visible, hence the ImMax(7,items_count)
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (ImGui::BeginPopup(popup_id)) {

		// the radial menu can be called either by a mouse click or by holding a certain key
		const bool usingMouse = mouse_button >= 0;
		const ImGuiStyle &style = ImGui::GetStyle();
		const float psize = size + padding * 2;
		const float RADIUS_MIN = 0.55 * psize;
		const float RADIUS_MAX = 2.4 * psize;
		const float RADIUS_INTERACT_MIN = RADIUS_MIN;
		const char *hovered_tooltip = nullptr;
		ImVec2 hovered_coord;
		ImVec2 drag_delta;
		if (usingMouse) {
			drag_delta = ImVec2(ImGui::GetIO().MousePos.x - center.x, ImGui::GetIO().MousePos.y - center.y);
		} else {
			const float length = (RADIUS_MIN + RADIUS_MAX) / 2;
			drag_delta = ImVec2(-horizontalSelection->GetValue() * length, -verticalSelection->GetValue() * length);
		}
		const float drag_dist2 = drag_delta.x * drag_delta.x + drag_delta.y * drag_delta.y;
		const int ITEMS_MIN = 2;
		const float border_inout = 0.3 * psize;
		const float border_thickness = 0.1 * psize;
		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		draw_list->PushClipRectFullScreen();
		draw_list->PathArcTo(center, (RADIUS_MIN + RADIUS_MAX) * 0.5f, 0.0f, IM_PI * 2.0f * 0.99f); // FIXME: 0.99f look like full arc with closed thick stroke has a bug now
		draw_list->PathStroke(bgCol, RADIUS_MAX - RADIUS_MIN, ImDrawFlags_Closed);

		const float item_arc_span = 2 * IM_PI / ImMax<int>(ITEMS_MIN, tex_ids.size());
		float drag_angle = atan2f(drag_delta.y, drag_delta.x);
		if (drag_angle < -0.5f * item_arc_span)
			drag_angle += 2.0f * IM_PI;

		int item_n = 0;
		assert(tex_ids.size() == tooltips.size() && tooltips.size() == colors.size());
		for (ImTextureID tex_id : tex_ids) {
			const char *tooltip = tooltips.at(item_n);
			const float inner_spacing = style.ItemInnerSpacing.x / RADIUS_MIN / 2;
			const float item_inner_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing);
			const float item_inner_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing);
			const float item_outer_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing * (RADIUS_MIN / RADIUS_MAX) * 2.0);
			const float item_outer_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing * (RADIUS_MIN / RADIUS_MAX) * 2.0);

			bool hovered = false;
			if (drag_dist2 >= RADIUS_INTERACT_MIN * RADIUS_INTERACT_MIN) {
				if (drag_angle >= item_inner_ang_min && drag_angle < item_inner_ang_max)
					hovered = true;
			}

			int arc_segments = static_cast<int>((64 * item_arc_span / (2 * IM_PI))) + 1;
			draw_list->_PathArcToN(center, RADIUS_MAX - border_inout, item_outer_ang_min, item_outer_ang_max, arc_segments);
			draw_list->_PathArcToN(center, RADIUS_MIN + border_inout, item_inner_ang_max, item_inner_ang_min, arc_segments);
			draw_list->PathFillConcave(hovered ? itemHoveredCol : itemBgCol);

			if (hovered) {
				// draw outer / inner extra segments
				draw_list->PathArcTo(center, RADIUS_MAX - border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(itemHoveredCol, border_thickness);
				draw_list->PathArcTo(center, RADIUS_MIN + border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(itemHoveredCol, border_thickness);
			}
			ImVec2 text_size = ImVec2(size, size);
			ImVec2 text_pos = ImVec2(
				center.x + cosf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.x * 0.5f,
				center.y + sinf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.y * 0.5f);
			draw_list->AddImage(tex_id, text_pos, ImVec2(text_pos.x + size, text_pos.y + size), uvs[item_n].first, uvs[item_n].second, colors[item_n]);
			ImGui::SameLine();
			if (hovered) {
				ret = item_n + 1;
				if (usingMouse) {
					ImGui::SetTooltip("%s", tooltip);
				} else {
					// draw custom text, since imgui only draws a tooltip over the mouse cursor
					// draw the text after the loop, otherwise it may be overlapped in subsequent iterations
					hovered_tooltip = tooltip;
					hovered_coord = ImVec2(text_pos.x + size, text_pos.y + size);
				}
			}
			item_n++;
		}

		if (hovered_tooltip) {
			draw_list->AddText(hovered_coord, IM_COL32_WHITE, hovered_tooltip);
		}
		draw_list->PopClipRect();

		ImGui::EndPopup();
	} else {
		// Output("WARNING: RadialPopupSelectMenu BeginPopup failed: %s\n", popup_id);
	}
	ImGui::PopStyleColor(3);
	return ret;
}

bool Draw::CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	const ImGuiID id = window->GetID("circularslider");
	draw_list->AddCircle(center, 17, ImColor(100, 100, 100), 128, 12.0);
	draw_list->PathArcTo(center, 17, 0, M_PI * 2.0 * (*v - v_min) / (v_max - v_min));
	draw_list->PathStroke(ImColor(200, 200, 200), 12.0);
	ImRect grab_bb;
	return ImGui::SliderBehavior(ImRect(center.x - 17, center.y - 17, center.x + 17, center.y + 17),
		id, ImGuiDataType_Float, v, &v_min, &v_max, "%.4f", ImGuiSliderFlags_None, &grab_bb);
}

void Draw::ThrustIndicator(ImGuiID id, float diameter, const vector3d &thrust)
{
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImGuiStyle &style = ImGui::GetStyle();

	const float radius = diameter * 0.5f;
	const float thickness = style.FrameBorderSize;
	const float offset = 0.5f * thickness;
	// radius of elements in the inner circle, leaving a small gap between their edge and the outer ring.
	const float inner_radius = radius - thickness * 2.f;

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const ImVec2 center = pos + ImVec2(diameter * 0.5f, diameter * 0.5f);
	const ImRect bb (pos, pos + ImVec2(diameter, diameter));

	ImGui::ItemSize(bb, 0.f);
	if (!ImGui::ItemAdd(bb, id))
		return;

	bool hovered = ImGui::IsItemHovered() && ImLengthSqr(ImGui::GetIO().MousePos - center) <= radius * radius;
	if (!hovered) {
		ImGui::GetCurrentContext()->LastItemData.StatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;
	}

	const ImU32 col_bg = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	const ImU32 col_ring = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
	const ImU32 col_thr = ImGui::GetColorU32(ImGuiCol_SliderGrab);

	ImDrawList *dl = ImGui::GetWindowDrawList();

	const auto drawThrustRing = [&](const ImVec2 &offset, float base_angle, float thrust) {
		if (thrust > 0.01) {
			// offset already carries the 0.5*thickness term, so just subtract thickess from radius
			dl->PathArcTo(center + offset, radius - thickness, base_angle - M_PI_4 * thrust, base_angle + M_PI_4 * thrust);
			dl->PathStroke(col_thr, thickness);
		}
	};

	dl->AddCircleFilled(center, radius - offset, col_bg);
	dl->AddCircle(center, radius - offset, col_ring, 0.f, thickness);

	// vertical thrust options
	drawThrustRing(ImVec2(0,  offset),  M_PI_2, fmax(0.f,  thrust.y));
	drawThrustRing(ImVec2(0, -offset), -M_PI_2, fmax(0.f, -thrust.y));

	// horizontal thrust options
	drawThrustRing(ImVec2( offset, 0), 0,    fmax(0.f,  thrust.x));
	drawThrustRing(ImVec2(-offset, 0), M_PI, fmax(0.f, -thrust.x));

	if (thrust.z > 0.01) {
		float circle_thickness = thrust.z * inner_radius * 0.75f;
		float circle_radius = inner_radius - circle_thickness * 0.5f;
		// add 0.5f to match the smooth antialiasing of AddCircleFilled
		dl->AddCircle(center, circle_radius + 0.5f, col_thr, 32, circle_thickness);
	}

	if (thrust.z < -0.01) {
		dl->AddCircleFilled(center, inner_radius * -thrust.z, col_thr, 32);
	}

}

void Draw::CircleIndicator(ImGuiID id, float diameter, const char *label, const char *unit, float value, float value_inv, float phase)
{
	constexpr float M_TAU = 2.0 * M_PI;

	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImGuiStyle &style = ImGui::GetStyle();

	const float radius = diameter * 0.5f;
	const float thickness = style.FrameBorderSize;
	const float offset = 0.5f * thickness;
	const float outer_radius = radius - offset;
	const float inner_radius = radius - thickness * 1.5;

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const ImVec2 center = pos + ImVec2(radius, radius);
	const ImRect bb (pos, pos + ImVec2(diameter, diameter));

	ImGui::ItemSize(bb, 0.f);
	if (!ImGui::ItemAdd(bb, id))
		return;

	bool hovered = ImGui::IsItemHovered() && ImLengthSqr(ImGui::GetIO().MousePos - center) <= radius * radius;
	if (!hovered) {
		ImGui::GetCurrentContext()->LastItemData.StatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;
	}

	ImFont *font = ImGui::GetFont();

	const ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
	const ImU32 col_bg = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	const ImU32 col_ring = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
	const ImU32 col_val1 = ImGui::GetColorU32(ImGuiCol_SliderGrab);
	const ImU32 col_val2 = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);

	ImDrawList *dl = ImGui::GetWindowDrawList();

	// Draw the center circle and outer ring
	dl->AddCircleFilled(center, outer_radius, col_bg);
	dl->AddCircle(center, outer_radius, col_ring, 0.f, thickness);

	if (value_inv >= 0.01) {
		// Draw the "limiter" dial under the "active" dial
		dl->PathArcTo(center, outer_radius, phase, phase + value_inv * M_TAU, 32);
		dl->PathStroke(col_val2, thickness);
	}

	if (value >= 0.01) {
		// Draw the "active" dial
		dl->PathArcTo(center, outer_radius, phase - value * M_TAU, phase, 32);
		dl->PathStroke(col_val1, thickness);
	}

	const auto computeTextScaleFactor = [&](ImVec2 size, float rad) -> float {
		return sqrt(size.x * size.x + size.y * size.y) / rad;
	};

	ImVec2 label_size = ImGui::CalcTextSize(label);
	float descent = ImGui::GetFontBaked()->Descent; // descent is negative

	if (unit) {
		// Draw the value and unit text, sizing both to fit within the upper and lower halves of the circle
		float scale_factor_inv = computeTextScaleFactor(ImVec2(label_size.x * 0.5f, label_size.y + descent), inner_radius);
		label_size /= scale_factor_inv;
		descent /= scale_factor_inv;
		dl->AddText(font, label_size.y, center - ImVec2(label_size.x * 0.5f, label_size.y + descent), col_text, label);

		float unit_voffset = descent - 2.f;

		ImVec2 unit_size = ImGui::CalcTextSize(unit);
		unit_size /= computeTextScaleFactor(ImVec2(unit_size.x * 0.5f, unit_size.y), fmin(inner_radius * 0.8, inner_radius + unit_voffset));
		dl->AddText(font, unit_size.y, center - ImVec2(unit_size.x * 0.5f, unit_voffset), col_text, unit);
	} else {
		// Draw only the value text centered in the indicator
		label_size /= computeTextScaleFactor(label_size * 0.5f, inner_radius);
		dl->AddText(font, label_size.y, center - label_size * 0.5f, col_text, label);
	}

}

bool Draw::LowThrustButton(const char *id_string, const ImVec2 &size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg)
{
	PROFILE_SCOPED()
	std::string label = std::to_string(thrust_level);
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;
	const ImGuiID id = window->GetID(id_string);
	const ImVec2 label_size = ImGui::CalcTextSize(label.c_str(), NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	// if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
	//     pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2(static_cast<float>(frame_padding), static_cast<float>(frame_padding)) : style.FramePadding;
	const ImRect bb(pos, pos + size + padding * 2);
	const ImRect inner_bb(pos + padding, pos + padding + size);

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	// if (window->DC.ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0); // flags

	// Render
	const ImU32 col = ImGui::GetColorU32(static_cast<ImGuiCol>((hovered && held) ? ImGuiCol_ButtonActive : (hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button)));
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	const ImVec2 center = (inner_bb.Min + inner_bb.Max) / 2;
	float radius = (inner_bb.Max.x - inner_bb.Min.x) * 0.4;
	float thickness = 4;
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	if (bg_col.w > 0.0f)
		draw_list->AddRectFilled(inner_bb.Min, inner_bb.Max, ImGui::GetColorU32(bg_col));

	draw_list->PathArcTo(center, radius, 0, IM_PI * 2);
	draw_list->PathStroke(gauge_bg, thickness);

	draw_list->PathArcTo(center, radius, IM_PI, IM_PI + IM_PI * 2 * (thrust_level / 100.0));
	draw_list->PathStroke(gauge_fg, thickness);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label.c_str(), NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool Draw::ButtonImageSized(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &imgSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)
{
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	ImGui::PushID((void *)user_texture_id);
	const ImGuiID id = window->GetID("#image");
	ImGui::PopID();

	ImVec2 imgPadding = (size - imgSize) / 2;
	imgPadding.x = imgPadding.x < 0 || imgSize.x <= 0 ? 0 : imgPadding.x;
	imgPadding.y = imgPadding.y < 0 || imgSize.y <= 0 ? 0 : imgPadding.y;

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	const ImRect image_bb(window->DC.CursorPos + padding + imgPadding, window->DC.CursorPos + padding + size - imgPadding);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::RenderNavHighlight(bb, id);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, ImGui::GetColorU32(bg_col));
	window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, ImGui::GetColorU32(tint_col));

	return pressed;
}

Draw::DragChangeMode Draw::IncrementDrag(const char *label, double &v, float v_speed, double v_min, double v_max, const char *format, bool draw_progress_bar)
{
	PROFILE_SCOPED()

	// getting vars storage for given label, use label as id
	ImGui::PushID(ImGui::GetID(label));
	auto storage = ImGui::GetStateStorage();
	// getting permanent vars
	// this is used to speed up the change when the button is held down
	float inc = storage->GetFloat(ImGui::GetID("##inc"), 0.1f);
	float waiting = storage->GetFloat(ImGui::GetID("##waiting"), 0.0f);
	bool typing = storage->GetBool(ImGui::GetID("##typing"), false);
	bool changed = false;

	float w = ImGui::CalcItemWidth();		   // full width of the widget
	float h = ImGui::GetFrameHeight();		   // full height of the widget
	ImVec2 pos = ImGui::GetCursorPos();		   // relative to window, for buttons
	ImVec2 spos = ImGui::GetCursorScreenPos(); // relative to screen, for lines

	ImGui::PushButtonRepeat(true); // can hold button to continue increment

	bool LeftButtonClick{}, LeftButtonHold{}, RightButtonHold{}, RightButtonClick{};
	if (draw_progress_bar) {
		float ratio = Clamp((v - v_min) / (v_max - v_min), 0.0, 1.0);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
		ImGui::ProgressBar(ratio, ImVec2(w, 0), "");
		ImGui::PopStyleColor();
		ImGui::SetCursorPos(pos);
	} else {
		// fill the background because the drag is drawn transparent
		auto col = ImGui::GetColorU32(ImGuiCol_FrameBg);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(spos.x, spos.y + h / 2 - 0.5), ImVec2(spos.x + w, spos.y + h / 2 - 0.5), col, h);
	}

	bool arrowsHovered{};
	if (!typing) {
		// draw buttons before the drag so that the click event gets to them
		LeftButtonClick = ImGui::ArrowButton("##left", ImGuiDir_Left); // this can be false, even when the button is holded
		arrowsHovered = arrowsHovered || ImGui::IsItemHovered();
		LeftButtonHold = ImGui::IsItemActive(); // if the button is holded, this is always true
		if (LeftButtonClick && waiting < inc) {
			v -= (double)ceilf(inc);
			changed = true;
			if (v < v_min) {
				v = v_min;
			}
		}
		int bw = ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x; // the width of the left button, used to place the right button properly
		ImGui::SetCursorPos(ImVec2(pos.x + w - bw, pos.y));
		RightButtonClick = ImGui::ArrowButton("##right", ImGuiDir_Right);
		arrowsHovered = arrowsHovered || ImGui::IsItemHovered();
		RightButtonHold = ImGui::IsItemActive();
		if (RightButtonClick && waiting < inc) {
			v += (double)ceilf(inc);
			changed = true;
			if (v > v_max) {
				v = v_max;
			}
		}
		ImGui::SetCursorPos(pos);
	}

	ImVec2 mousePos = ImGui::GetIO().MousePos;

	// we need to know this before we draw a scalar
	bool hovered = mousePos.x > spos.x && mousePos.y > spos.y && mousePos.x < spos.x + w && mousePos.y < spos.y + h;

	// this is used to remove format artifacts when editing from the keyboard, just the underlying number
	const char *raw_format = "%.0f";
	const char **f = hovered && !arrowsHovered && ImGui::IsMouseDoubleClicked(0) ? &raw_format : &format;

	ImGui::SetNextItemWidth(w);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
	changed = ImGui::DragScalar(label, ImGuiDataType_Double, &v, v_speed, &v_min, &v_max, *f, ImGuiSliderFlags_AlwaysClamp) || changed;
	ImGui::PopStyleColor();
	ImGui::PopButtonRepeat();

	typing = typing ? ImGui::IsItemActive() : hovered && !arrowsHovered && ImGui::IsMouseDoubleClicked(0);

	if (hovered && ImGui::GetIO().MouseWheel) {
		changed = true;
		v += ImGui::GetIO().MouseWheel;
	}

	v = Clamp(v, v_min, v_max);

	// this code makes the increment acceleration
	if (!typing) {
		if (RightButtonClick || LeftButtonClick) {
			if (waiting < inc) {
				inc *= 1.1f;					// acceleration of the increment
				if (inc > 123.0f) inc = 123.0f; // max increment in one frame
				waiting = 1.0f;					// x10 of start increment -> 10 frames to wait for the first increment
			} else
				waiting -= inc;
		} else if (!RightButtonHold && !LeftButtonHold) {
			// nothing touched, reset
			inc = 0.1f;
			waiting = 0.0f; // because first click always increment
		}
	}

	// remember permanent vars
	storage->SetFloat(ImGui::GetID("##inc"), inc);
	storage->SetFloat(ImGui::GetID("##waiting"), waiting);
	storage->SetBool(ImGui::GetID("##typing"), typing);
	ImGui::PopID();

	using DCM = Draw::DragChangeMode;
	return changed ? typing ? DCM::CHANGED_BY_TYPING : DCM::CHANGED : DCM::NOT_CHANGED;
}

bool Draw::GlyphButton(const char *str_id, const char *glyph, const ImVec2 &size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);
    const ImVec2 glyph_size = ImGui::CalcTextSize(glyph, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, glyph_size.x + style.FramePadding.x * 2.0f, glyph_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, glyph, NULL, &glyph_size, style.ButtonTextAlign, &bb);

    return pressed;
}

void Draw::BeginHorizontalGroup()
{
	ImGui::BeginGroup();
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;
}

void Draw::EndHorizontalGroup()
{
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Vertical;
	ImGui::EndGroup();
}
