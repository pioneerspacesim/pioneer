// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGui.h"
#include "imgui/imgui.h"

// to get ImVec2 + ImVec2
#define IMGUI_DEFINE_MATH_OPERATORS true
#include "imgui/imgui_internal.h"

int PiGui::RadialPopupSelectMenu(const ImVec2 &center, std::string popup_id, int mouse_button, std::vector<ImTextureID> tex_ids, std::vector<std::pair<ImVec2, ImVec2>> uvs, unsigned int size, std::vector<std::string> tooltips)
{
	PROFILE_SCOPED()
	// return:
	// 0 - n for item selected
	// -1 for nothing chosen, but menu open
	// -2 for menu closed without an icon chosen
	// -3 for menu not open
	int ret = -3;

	// FIXME: Missing a call to query if Popup is open so we can move the PushStyleColor inside the BeginPopupBlock (e.g. IsPopupOpen() in imgui.cpp)
	// FIXME: Our PathFill function only handle convex polygons, so we can't have items spanning an arc too large else inner concave edge artifact is too visible, hence the ImMax(7,items_count)
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	if (ImGui::BeginPopup(popup_id.c_str())) {
		ret = -1;
		const ImVec2 drag_delta = ImVec2(ImGui::GetIO().MousePos.x - center.x, ImGui::GetIO().MousePos.y - center.y);
		const float drag_dist2 = drag_delta.x * drag_delta.x + drag_delta.y * drag_delta.y;

		const ImGuiStyle &style = ImGui::GetStyle();
		const float RADIUS_MIN = 20.0f;
		const float RADIUS_MAX = 90.0f;
		const float RADIUS_INTERACT_MIN = 20.0f;
		const int ITEMS_MIN = 4;
		const float border_inout = 12.0f;
		const float border_thickness = 4.0f;
		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		draw_list->PushClipRectFullScreen();
		draw_list->PathArcTo(center, (RADIUS_MIN + RADIUS_MAX) * 0.5f, 0.0f, IM_PI * 2.0f * 0.99f, 64); // FIXME: 0.99f look like full arc with closed thick stroke has a bug now
		draw_list->PathStroke(ImColor(18, 44, 67, 210), true, RADIUS_MAX - RADIUS_MIN);

		const float item_arc_span = 2 * IM_PI / ImMax<int>(ITEMS_MIN, tex_ids.size());
		float drag_angle = atan2f(drag_delta.y, drag_delta.x);
		if (drag_angle < -0.5f * item_arc_span)
			drag_angle += 2.0f * IM_PI;

		int item_hovered = -1;
		int item_n = 0;
		for (ImTextureID tex_id : tex_ids) {
			const char *tooltip = tooltips.at(item_n).c_str();
			const float inner_spacing = style.ItemInnerSpacing.x / RADIUS_MIN / 2;
			const float item_inner_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing);
			const float item_inner_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing);
			const float item_outer_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing * (RADIUS_MIN / RADIUS_MAX));
			const float item_outer_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing * (RADIUS_MIN / RADIUS_MAX));

			bool hovered = false;
			if (drag_dist2 >= RADIUS_INTERACT_MIN * RADIUS_INTERACT_MIN) {
				if (drag_angle >= item_inner_ang_min && drag_angle < item_inner_ang_max)
					hovered = true;
			}
			bool selected = false;

			int arc_segments = static_cast<int>((64 * item_arc_span / (2 * IM_PI))) + 1;
			draw_list->PathArcTo(center, RADIUS_MAX - border_inout, item_outer_ang_min, item_outer_ang_max, arc_segments);
			draw_list->PathArcTo(center, RADIUS_MIN + border_inout, item_inner_ang_max, item_inner_ang_min, arc_segments);

			draw_list->PathFillConvex(hovered ? ImColor(102, 147, 189) : selected ? ImColor(48, 81, 111) : ImColor(48, 81, 111));
			if (hovered) {
				// draw outer / inner extra segments
				draw_list->PathArcTo(center, RADIUS_MAX - border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(ImColor(102, 147, 189), false, border_thickness);
				draw_list->PathArcTo(center, RADIUS_MIN + border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(ImColor(102, 147, 189), false, border_thickness);
			}
			ImVec2 text_size = ImVec2(size, size);
			ImVec2 text_pos = ImVec2(
				center.x + cosf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.x * 0.5f,
				center.y + sinf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.y * 0.5f);
			draw_list->AddImage(tex_id, text_pos, ImVec2(text_pos.x + size, text_pos.y + size), uvs[item_n].first, uvs[item_n].second);
			ImGui::SameLine();
			if (hovered) {
				item_hovered = item_n;
				ImGui::SetTooltip("%s", tooltip);
			}
			item_n++;
		}
		draw_list->PopClipRect();

		if (ImGui::IsMouseReleased(mouse_button)) {
			ImGui::CloseCurrentPopup();
			if (item_hovered == -1)
				ret = -2;
			else
				ret = item_hovered;
		}
		ImGui::EndPopup();
	} else {
		// Output("WARNING: RadialPopupSelectMenu BeginPopup failed: %s\n", popup_id.c_str());
	}
	ImGui::PopStyleColor(3);
	return ret;
}

bool PiGui::CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	const ImGuiID id = window->GetID("circularslider");
	draw_list->AddCircle(center, 17, ImColor(100, 100, 100), 128, 12.0);
	draw_list->PathArcTo(center, 17, 0, M_PI * 2.0 * (*v - v_min) / (v_max - v_min), 64);
	draw_list->PathStroke(ImColor(200, 200, 200), false, 12.0);
	ImRect grab_bb;
	return ImGui::SliderBehavior(ImRect(center.x - 17, center.y - 17, center.x + 17, center.y + 17),
		id, ImGuiDataType_Float, v, &v_min, &v_max, "%.4f", 1.0, ImGuiSliderFlags_None, &grab_bb);
}

static void drawThrust(ImDrawList *draw_list, const ImVec2 &center, const ImVec2 &up, float value, const ImColor &fg, const ImColor &bg)
{
	PROFILE_SCOPED()
	float factor = 0.1; // how much to offset from center
	const ImVec2 step(up.x * 0.5, up.y * 0.5);
	const ImVec2 left(-step.y * (1.0 - factor), step.x * (1.0 - factor));
	const ImVec2 u(up.x * (1.0 - factor), up.y * (1.0 - factor));
	const ImVec2 c(center + ImVec2(u.x * factor, u.y * factor));
	const ImVec2 right(-left.x, -left.y);
	const ImVec2 leftmiddle = c + step + left;
	const ImVec2 rightmiddle = c + step + right;
	const ImVec2 bb_lowerright = c + right;
	const ImVec2 bb_upperleft = c + left + ImVec2(u.x * value, u.y * value);
	const ImVec2 lefttop = c + u + left;
	const ImVec2 righttop = c + u + right;
	const ImVec2 minimum(fmin(bb_upperleft.x, bb_lowerright.x), fmin(bb_upperleft.y, bb_lowerright.y));
	const ImVec2 maximum(fmax(bb_upperleft.x, bb_lowerright.x), fmax(bb_upperleft.y, bb_lowerright.y));
	ImVec2 points[] = { c, leftmiddle, lefttop, righttop, rightmiddle };
	draw_list->AddConvexPolyFilled(points, 5, bg);
	draw_list->PushClipRect(minimum - ImVec2(1, 1), maximum + ImVec2(1, 1));
	draw_list->AddConvexPolyFilled(points, 5, fg);
	draw_list->PopClipRect();
}

void PiGui::ThrustIndicator(const std::string &id_string, const ImVec2 &size_arg, const ImVec4 &thrust, const ImVec4 &velocity, const ImVec4 &bg_col, int frame_padding, ImColor vel_fg, ImColor vel_bg, ImColor thrust_fg, ImColor thrust_bg)
{
	PROFILE_SCOPED()
	ImGuiWindow *window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;
	const ImGuiID id = window->GetID(id_string.c_str());

	ImVec2 pos = window->DC.CursorPos;
	// if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
	//     pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(size_arg, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2(static_cast<float>(frame_padding), static_cast<float>(frame_padding)) : style.FramePadding;
	const ImRect bb(pos, pos + size + padding * 2);
	const ImRect inner_bb(pos + padding, pos + padding + size);

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return;

	// Render
	const ImU32 col = ImGui::GetColorU32(static_cast<ImGuiCol>(ImGuiCol_Button));
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	if (bg_col.w > 0.0f)
		draw_list->AddRectFilled(inner_bb.Min, inner_bb.Max, ImGui::GetColorU32(bg_col));
	const ImVec2 leftupper = inner_bb.Min;
	const ImVec2 rightlower = inner_bb.Max;
	const ImVec2 rightcenter((rightlower.x - leftupper.x) * 0.8 + leftupper.x, (rightlower.y + leftupper.y) / 2);
	const ImVec2 leftcenter((rightlower.x - leftupper.x) * 0.35 + leftupper.x, (rightlower.y + leftupper.y) / 2);
	const ImVec2 up(0, -std::abs(leftupper.y - rightlower.y) * 0.4);
	const ImVec2 left(-up.y, up.x);
	float thrust_fwd = fmax(thrust.z, 0);
	float thrust_bwd = fmax(-thrust.z, 0);
	float thrust_left = fmax(-thrust.x, 0);
	float thrust_right = fmax(thrust.x, 0);
	float thrust_up = fmax(-thrust.y, 0);
	float thrust_down = fmax(thrust.y, 0);
	// actual thrust
	drawThrust(draw_list, rightcenter, up, thrust_fwd, thrust_fg, thrust_bg);
	drawThrust(draw_list, rightcenter, ImVec2(-up.x, -up.y), thrust_bwd, thrust_fg, thrust_bg);
	drawThrust(draw_list, leftcenter, up, thrust_up, thrust_fg, thrust_bg);
	drawThrust(draw_list, leftcenter, ImVec2(-up.x, -up.y), thrust_down, thrust_fg, thrust_bg);
	drawThrust(draw_list, leftcenter, left, thrust_left, thrust_fg, thrust_bg);
	drawThrust(draw_list, leftcenter, ImVec2(-left.x, -left.y), thrust_right, thrust_fg, thrust_bg);
	// forward/back velocity
	draw_list->AddLine(rightcenter + up, rightcenter - up, vel_bg, 3);
	draw_list->AddLine(rightcenter, rightcenter - up * velocity.z, vel_fg, 3);
	// left/right velocity
	draw_list->AddLine(leftcenter + left, leftcenter - left, vel_bg, 3);
	draw_list->AddLine(leftcenter, leftcenter + left * velocity.x, vel_fg, 3);
	// up/down velocity
	draw_list->AddLine(leftcenter + up, leftcenter - up, vel_bg, 3);
	draw_list->AddLine(leftcenter, leftcenter + up * velocity.y, vel_fg, 3);
	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();
}

bool PiGui::LowThrustButton(const char *id_string, const ImVec2 &size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg)
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

	draw_list->PathArcTo(center, radius, 0, IM_PI * 2, 16);
	draw_list->PathStroke(gauge_bg, false, thickness);

	draw_list->PathArcTo(center, radius, IM_PI, IM_PI + IM_PI * 2 * (thrust_level / 100.0), 16);
	draw_list->PathStroke(gauge_fg, false, thickness);
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
bool PiGui::ButtonImageSized(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &imgSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)
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

void PiGui::IncrementDrag(const std::string &label, int &v, const int v_min, const int v_max, const std::string &format)
{
	PROFILE_SCOPED()
	// getting vars storage for given label, use label as id
	ImGui::PushID(ImGui::GetID(label.c_str()));
	auto storage = ImGui::GetStateStorage();
	// getting "static" vars
	float inc = storage->GetFloat(ImGui::GetID("##inc"), 0.1f);
	float waiting = storage->GetFloat(ImGui::GetID("##waiting"), 0.0f);

	// fill bar color
	const ImU32 col = ImGui::GetColorU32(ImGuiCol_FrameBgActive);

	float w = ImGui::CalcItemWidth();		   // full width of the widget
	float h = ImGui::GetFrameHeight();		   // full height of the widget
	ImVec2 pos = ImGui::GetCursorPos();		   // relative to window, for buttons
	ImVec2 spos = ImGui::GetCursorScreenPos(); // relative to screen, for lines

	// draw thick line
	ImGui::GetWindowDrawList()->AddLine(ImVec2(spos.x, spos.y + h / 2 - 0.5), ImVec2(spos.x + w / v_max * (v), spos.y + h / 2 - 0.5), col, h);
	// draw buttons before the drag so that the click event gets to them
	ImGui::PushButtonRepeat(true);										// can hold button to continue increment
	bool LeftButtonClick = ImGui::ArrowButton("##left", ImGuiDir_Left); // this can be false, even when the button is holded
	bool LeftButtonHold = ImGui::IsItemActive();						// if the button is holded, this is always true
	if (LeftButtonClick && waiting < inc && (v -= std::ceil(inc)) <= v_min) v = v_min;
	int bw = ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x; // the width of the left button, used to place the right button properly
	ImGui::SetCursorPos(ImVec2(pos.x + w - bw, pos.y));
	bool RightButtonClick = ImGui::ArrowButton("##right", ImGuiDir_Right);
	bool RightButtonHold = ImGui::IsItemActive();
	if (RightButtonClick && waiting < inc && (v += std::ceil(inc)) >= v_max)
		v = v_max;
	ImGui::SetCursorPos(pos);
	ImGui::SetNextItemWidth(w);
	ImGui::DragInt(label.c_str(), &v, v_max / w / 0.7, v_min, v_max, format.c_str());
	ImGui::PopButtonRepeat();

	// if user manually entered a number out of range
	v = Clamp(v, v_min, v_max);
	// this code makes the increment acceleration
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

	// remember "static" vars
	storage->SetFloat(ImGui::GetID("##inc"), inc);
	storage->SetFloat(ImGui::GetID("##waiting"), waiting);
	ImGui::PopID();
}
