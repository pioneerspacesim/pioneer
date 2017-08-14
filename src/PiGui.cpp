// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "graphics/opengl/TextureGL.h" // nasty, usage of GL is implementation specific
#include "PiGui.h"
// to get ImVec2 + ImVec2
#define IMGUI_DEFINE_MATH_OPERATORS true
#include "imgui/imgui_internal.h"

#include <stdio.h>
#include <string.h>
#include <float.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"


std::vector<Graphics::Texture*> PiGui::m_svg_textures;

static int to_keycode(int key) {
	if(key & SDLK_SCANCODE_MASK) {
		return (key & ~SDLK_SCANCODE_MASK) | 0x100;
	}
	return key;
}

static std::vector<std::pair<std::string,int>> keycodes
= { {"left", to_keycode(SDLK_LEFT) },
		{"right", to_keycode(SDLK_RIGHT)},
		{"up", to_keycode(SDLK_UP)},
		{"down", to_keycode(SDLK_DOWN)},
		{"escape", to_keycode(SDLK_ESCAPE)},
		{"f1", to_keycode(SDLK_F1)},
		{"f2", to_keycode(SDLK_F2)},
		{"f3", to_keycode(SDLK_F3)},
		{"f4", to_keycode(SDLK_F4)},
		{"f5", to_keycode(SDLK_F5)},
		{"f6", to_keycode(SDLK_F6)},
		{"f7", to_keycode(SDLK_F7)},
		{"f8", to_keycode(SDLK_F8)},
		{"f9", to_keycode(SDLK_F9)},
		{"f10", to_keycode(SDLK_F10)},
		{"f11", to_keycode(SDLK_F11)},
		{"f12", to_keycode(SDLK_F12)}
};

ImTextureID PiGui::RenderSVG(std::string svgFilename, int width, int height) {
	Output("nanosvg: %s %dx%d\n", svgFilename.c_str(), width, height);

	// // re-use existing texture if already loaded
	// for(auto strTex : m_svg_textures) {
	// 	if(strTex.first == svgFilename) {
	// 		// nasty bit as I invoke the TextureGL
	// 		Graphics::TextureGL *pGLTex = reinterpret_cast<Graphics::TextureGL*>(strTex.second);
	// 		Uint32 result = pGLTex->GetTexture();
	// 		Output("Re-used existing texture with id: %i\n", result);
	// 		return reinterpret_cast<void*>(result);
	// 	}
	// }

	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w;
	// size of each icon
	//	int size = 64;
	// 16 columns
	//	int W = 16*size;
	int W = width;
	// 16 rows
	//	int H = 16*size;
	int H = height;
	img = static_cast<unsigned char*>(malloc(W*H*4));
	memset(img, 0, W * H * 4);
	image = nsvgParseFromFile(svgFilename.c_str(), "px", 96.0f);
	if (image == NULL) {
		Error("Could not open SVG image.\n");
	}
	w = static_cast<int>(image->width);

	rast = nsvgCreateRasterizer();
	if (rast == NULL) {
		Error("Could not init rasterizer.\n");
	}

	if (img == NULL) {
		Error("Could not alloc image buffer.\n");
	}
	{
		float scale = double(W)/w;
		float tx = 0;
		float ty = 0;
		nsvgRasterize(rast, image, tx, ty, scale, img, W, H, W*4);
	}
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);
	return makeTexture(img, W, H);
}

ImFont *PiGui::GetFont(const std::string &name, int size) {
	auto iter = m_fonts.find(std::make_pair(name, size));
	if(iter != m_fonts.end())
		return iter->second;
	//	Output("GetFont: adding font %s at %i on demand\n", name.c_str(), size);
	ImFont *font = AddFont(name, size);

	return font;
}

void PiGui::AddGlyph(ImFont *font, unsigned short glyph) {
	// range glyph..glyph
	auto iter = m_im_fonts.find(font);
	if(iter == m_im_fonts.end()) {
		Error("Cannot find font instance for ImFont %p\n", font);
		assert(false);
	}
	auto pifont_iter = m_pi_fonts.find(iter->second);
	if(pifont_iter == m_pi_fonts.end()) {
		Error("No registered PiFont for name %s size %i\n", iter->second.first.c_str(), iter->second.second);
		assert(false);
	}
	PiFont &pifont = pifont_iter->second;
	for(PiFace &face : pifont.faces()) {
		if(face.containsGlyph(glyph)) {
			face.addGlyph(glyph);
			m_should_bake_fonts = true;
			return;
		}
	}
	Error("No face in font %s handles glyph %i\n", pifont.name().c_str(), glyph);
}

ImFont *PiGui::AddFont(const std::string &name, int size) {
	auto iter = m_font_definitions.find(name);
	if(iter == m_font_definitions.end()) {
		Error("No font definition with name %s\n", name.c_str());
		assert(false);
	}
	auto existing = m_fonts.find(std::make_pair(name,size));
	if(existing != m_fonts.end()) {
		Error("Font %s already exists at size %i\n", name.c_str(), size);
		assert(false);
	}

	PiFont &pifont = iter->second;
	pifont.setPixelsize(size);
	pifont.faces().back().addGlyph(0x20); // only add space
	m_pi_fonts[std::make_pair(name,size)] = pifont;

	m_should_bake_fonts = true;

	return m_fonts[std::make_pair(name,size)];
}

void PiGui::RefreshFontsTexture() {
	// TODO: fix this, do the right thing, don't just re-create *everything* :)
	ImGui::GetIO().Fonts->Build();
	ImGui_ImplSdlGL3_CreateDeviceObjects();
}

void PiGui::Init(SDL_Window *window) {
	m_handlers.Unref();

	lua_State *l = Lua::manager->GetLuaState();
	lua_newtable(l);
	m_handlers = LuaRef(l, -1);

	lua_newtable(l);
	m_keys = LuaRef(l, -1);
	LuaTable keys(l, -1);
	for(auto p : keycodes) {
		keys.Set(p.first, p.second);
	}

	switch(Pi::renderer->GetRendererType())
		{
		default:
		case Graphics::RENDERER_DUMMY:
			Error("RENDERER_DUMMY is not a valid renderer, aborting.");
			return;
		case Graphics::RENDERER_OPENGL_21:
			ImGui_ImplSdl_Init(window);
			break;
		case Graphics::RENDERER_OPENGL_3x:
			ImGui_ImplSdlGL3_Init(window);
			break;
		}

	ImGuiIO &io = ImGui::GetIO();

	std::string imguiIni = FileSystem::JoinPath(FileSystem::GetUserDir(), "imgui.ini");
	// this will be leaked, not sure how to deal with it properly in imgui...
	char *ioIniFilename = new char[imguiIni.size() + 1];
	std::strncpy(ioIniFilename, imguiIni.c_str(), imguiIni.size() + 1);
	io.IniFilename = ioIniFilename;

}

int PiGui::RadialPopupSelectMenu(const ImVec2& center, std::string popup_id, std::vector<ImTextureID> tex_ids, std::vector<std::pair<ImVec2,ImVec2>> uvs, unsigned int size, std::vector<std::string> tooltips)
{
	int ret = -1;

	// FIXME: Missing a call to query if Popup is open so we can move the PushStyleColor inside the BeginPopupBlock (e.g. IsPopupOpen() in imgui.cpp)
	// FIXME: Our PathFill function only handle convex polygons, so we can't have items spanning an arc too large else inner concave edge artifact is too visible, hence the ImMax(7,items_count)
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0,0,0,0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
	if (ImGui::BeginPopup(popup_id.c_str())) {
		const ImVec2 drag_delta = ImVec2(ImGui::GetIO().MousePos.x - center.x, ImGui::GetIO().MousePos.y - center.y);
		const float drag_dist2 = drag_delta.x*drag_delta.x + drag_delta.y*drag_delta.y;

		const ImGuiStyle& style = ImGui::GetStyle();
		const float RADIUS_MIN = 20.0f;
		const float RADIUS_MAX = 90.0f;
		const float RADIUS_INTERACT_MIN = 20.0f;
		const int ITEMS_MIN = 5;
		const float border_inout = 12.0f;
		const float border_thickness = 4.0f;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->PushClipRectFullScreen();
		draw_list->PathArcTo(center, (RADIUS_MIN + RADIUS_MAX)*0.5f, 0.0f, IM_PI*2.0f*0.99f, 64);   // FIXME: 0.99f look like full arc with closed thick stroke has a bug now
		draw_list->PathStroke(ImColor(18,44,67,210), true, RADIUS_MAX - RADIUS_MIN);

		const float item_arc_span = 2*IM_PI / ImMax(ITEMS_MIN, tex_ids.size());
		float drag_angle = atan2f(drag_delta.y, drag_delta.x);
		if (drag_angle < -0.5f*item_arc_span)
			drag_angle += 2.0f*IM_PI;

		int item_hovered = -1;
		int item_n = 0;
		for(ImTextureID tex_id : tex_ids) {
			const char* tooltip = tooltips.at(item_n).c_str();
			const float inner_spacing = style.ItemInnerSpacing.x / RADIUS_MIN / 2;
			const float item_inner_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing);
			const float item_inner_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing);
			const float item_outer_ang_min = item_arc_span * (item_n - 0.5f + inner_spacing * (RADIUS_MIN / RADIUS_MAX));
			const float item_outer_ang_max = item_arc_span * (item_n + 0.5f - inner_spacing * (RADIUS_MIN / RADIUS_MAX));

			bool hovered = false;
			if (drag_dist2 >= RADIUS_INTERACT_MIN*RADIUS_INTERACT_MIN) {
				if (drag_angle >= item_inner_ang_min && drag_angle < item_inner_ang_max)
					hovered = true;
			}
			bool selected = false;

			int arc_segments = static_cast<int>((64 * item_arc_span / (2*IM_PI))) + 1;
			draw_list->PathArcTo(center, RADIUS_MAX - border_inout, item_outer_ang_min, item_outer_ang_max, arc_segments);
			draw_list->PathArcTo(center, RADIUS_MIN + border_inout, item_inner_ang_max, item_inner_ang_min, arc_segments);

			draw_list->PathFill(hovered ? ImColor(102,147,189) : selected ? ImColor(48,81,111) : ImColor(48,81,111));
			if(hovered) {
				// draw outer / inner extra segments
				draw_list->PathArcTo(center, RADIUS_MAX - border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(ImColor(102,147,189), false, border_thickness);
				draw_list->PathArcTo(center, RADIUS_MIN + border_thickness, item_outer_ang_min, item_outer_ang_max, arc_segments);
				draw_list->PathStroke(ImColor(102,147,189), false, border_thickness);
			}
			ImVec2 text_size = ImVec2(size, size);
			ImVec2 text_pos = ImVec2(
															 center.x + cosf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.x * 0.5f,
															 center.y + sinf((item_inner_ang_min + item_inner_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.y * 0.5f);
			draw_list->AddImage(tex_id, text_pos, ImVec2(text_pos.x+size,text_pos.y+size), uvs[item_n].first, uvs[item_n].second); ImGui::SameLine();
			if (hovered) {
				item_hovered = item_n;
				ImGui::SetTooltip("%s", tooltip);

			}
			item_n++;
		}
		draw_list->PopClipRect();

		if (ImGui::IsMouseReleased(1)) {
			ImGui::CloseCurrentPopup();
			if(item_hovered == -1)
				ret = -2;
			else
				ret = item_hovered;
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(3);
	return ret;
}

bool PiGui::CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImGuiID id = window->GetID("circularslider");
	draw_list->AddCircle(center, 17, ImColor(100, 100, 100), 128, 12.0);
	draw_list->PathArcTo(center, 17, 0, M_PI * 2.0 * (*v - v_min) / (v_max - v_min), 64);
	draw_list->PathStroke(ImColor(200,200,200), false, 12.0);
	return ImGui::SliderBehavior(ImRect(center.x - 17, center.y - 17, center.x + 17, center.y + 17), id, v, v_min, v_max, 1.0, 4);
}

bool PiGui::ProcessEvent(SDL_Event *event)
{
	switch(Pi::renderer->GetRendererType())
		{
		default:
		case Graphics::RENDERER_DUMMY:
			Error("RENDERER_DUMMY is not a valid renderer, aborting.");
			break;
		case Graphics::RENDERER_OPENGL_21:
			ImGui_ImplSdl_ProcessEvent(event);
			break;
		case Graphics::RENDERER_OPENGL_3x:
			ImGui_ImplSdlGL3_ProcessEvent(event);
			break;
		}
	return false;
}

void *PiGui::makeTexture(unsigned char *pixels, int width, int height)
{
	// this is not very pretty code and uses the Graphics::TextureGL class directly
	// Texture descriptor defines the size, type.
	// Gone for LINEAR_CLAMP here and RGBA like the original code
	const vector2f texSize(1.0f, 1.0f);
	const vector2f dataSize(width, height);
	const Graphics::TextureDescriptor texDesc(Graphics::TEXTURE_RGBA_8888,
																						dataSize, texSize, Graphics::LINEAR_CLAMP,
																						false, false, false, 0, Graphics::TEXTURE_2D);
	// Create the texture, calling it via renderer directly avoids the caching call of TextureBuilder
	// However interestingly this gets called twice which would have been a WIN for the TextureBuilder :/
	Graphics::Texture *pTex = Pi::renderer->CreateTexture(texDesc);
	// Update it with the actual pixels, this is a two step process due to legacy code
	pTex->Update(pixels, dataSize, Graphics::TEXTURE_RGBA_8888);
	// nasty bit as I invoke the TextureGL
	Graphics::OGL::TextureGL *pGLTex = reinterpret_cast<Graphics::OGL::TextureGL*>(pTex);
	Uint32 result = pGLTex->GetTextureID();
	m_svg_textures.push_back( pTex ); // store for cleanup later
	return reinterpret_cast<void*>(result);
}

void PiGui::EndFrame() {
	ImGui::EndFrame();
}

void PiGui::NewFrame(SDL_Window *window) {
	// Ask ImGui to hide OS cursor if GUI is not being drawn:
	// it will do this if MouseDrawCursor is true. After the frame
	// is created, we set the actual cursor draw state.
	if (!Pi::DrawGUI)
		{
			ImGui::GetIO().MouseDrawCursor = true;
		}
	switch(Pi::renderer->GetRendererType())
		{
		default:
		case Graphics::RENDERER_DUMMY:
			Error("RENDERER_DUMMY is not a valid renderer, aborting.");
			return;
		case Graphics::RENDERER_OPENGL_21:
			ImGui_ImplSdl_NewFrame(window);
			break;
		case Graphics::RENDERER_OPENGL_3x:
			ImGui_ImplSdlGL3_NewFrame(window);
			break;
		}
	Pi::renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
	ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	if(Pi::DoingMouseGrab() || !Pi::DrawGUI)
		{
			ImGui::GetIO().MouseDrawCursor = false;
		}
	else
		{
			ImGui::GetIO().MouseDrawCursor = true;
		}
}
void PiGui::Render(double delta, std::string handler) {
	ScopedTable t(m_handlers);
	if(t.Get<bool>(handler)) {
		t.Call<bool>(handler, delta);
		Pi::renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
	}
	// Explicitly end frame, to show tooltips. Otherwise, they are shown at the next NextFrame,
	// which might crash because the font atlas was rebuilt, and the old fonts were cached inside imgui.
	EndFrame();
	for(auto &iter : m_fonts) {
		ImFont *font = iter.second;
		// font might be nullptr, if it wasn't baked yet
		if(font && font->AreGlyphsMissing()) {
			//			Output("%s %i is missing glyphs.\n", iter.first.first.c_str(), iter.first.second);
			for(auto &glyph : font->MissingGlyphs()) {
				AddGlyph(font, glyph);
			}
			font->ResetMissingGlyphs();
		}
	}
	// Bake fonts *after* a frame is done, so the font atlas is not needed any longer
	if(m_should_bake_fonts) {
		BakeFonts();
	}
}

void PiGui::ClearFonts() {
	ImGuiIO &io = ImGui::GetIO();
	// TODO: should also release all glyph_ranges...
	m_fonts.clear();
	m_im_fonts.clear();
	io.Fonts->Clear();
}

void PiGui::BakeFont(const PiFont &font) {
	ImGuiIO &io = ImGui::GetIO();
	ImFont *imfont = nullptr;
	for(const PiFace &face : font.faces()) {
		ImFontConfig config;
		config.MergeMode = true;
		float size = font.pixelsize() * face.sizefactor();
		const std::string path = FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), face.ttfname());
		//		Output("- baking face %s at size %f\n", path.c_str(), size);
		face.sortUsedRanges();
		if(face.used_ranges().size() > 0) {
			unsigned short *glyph_ranges = new unsigned short[face.used_ranges().size() * 2 + 2];
			unsigned short *gr = glyph_ranges;
			for(auto &range : face.used_ranges()) {
				// 				Output("  - 0x%x .. 0x%x\n", range.first, range.second);
				*gr++ = range.first;
				*gr++ = range.second;
			}
			*gr++ = 0;
			*gr = 0;
			ImFont *f = io.Fonts->AddFontFromFileTTF(path.c_str(), size, imfont == nullptr ? nullptr : &config, glyph_ranges);
			assert(f);
			if(imfont != nullptr)
				assert(f == imfont);
			imfont = f;
		}
	}
	m_im_fonts[imfont] = std::make_pair(font.name(), font.pixelsize());
	// 	Output("setting %s %i to %p\n", font.name(), font.pixelsize(), imfont);
	m_fonts[std::make_pair(font.name(), font.pixelsize())] = imfont;
	if(imfont->AreGlyphsMissing()) {
		Output("WARNING: glyphs missing in shiny new font\n");
	}
	imfont->ResetMissingGlyphs();
}

void PiGui::BakeFonts() {
	//	Output("Baking fonts\n");

	m_should_bake_fonts = false;

	if(m_pi_fonts.size() == 0) {
		//		Output("No fonts to bake.\n");
		return;
	}

	ClearFonts();

	// first bake tooltip/default font
	BakeFont(m_pi_fonts[std::make_pair("pionillium", 14)]);

	for(auto &iter : m_pi_fonts) {
		// don't bake tooltip/default font again
		if(!(iter.first.first == "pionillium" && iter.first.second == 14))
			BakeFont(iter.second);
		//		Output("Fonts registered: %i\n", io.Fonts->Fonts.Size);
	}

	RefreshFontsTexture();
}

static void drawThrust(ImDrawList* draw_list, const ImVec2 &center, const ImVec2 &up, float value, const ImColor &fg, const ImColor &bg) {
	float factor = 0.1; // how much to offset from center
	const ImVec2 step(up.x * 0.5, up.y * 0.5);
	const ImVec2 left(-step.y * (1.0-factor), step.x * (1.0-factor));
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
	draw_list->AddConvexPolyFilled(points, 5, bg, false);
	draw_list->PushClipRect(minimum - ImVec2(1,1), maximum + ImVec2(1,1));
	draw_list->AddConvexPolyFilled(points, 5, fg, false);
	draw_list->PopClipRect();
}

void PiGui::ThrustIndicator(const std::string &id_string, const ImVec2& size_arg, const ImVec4& thrust, const ImVec4& velocity, const ImVec4 &bg_col, int frame_padding, ImColor vel_fg, ImColor vel_bg, ImColor thrust_fg, ImColor thrust_bg)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(id_string.c_str());

	ImVec2 pos = window->DC.CursorPos;
	// if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
	//     pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(size_arg, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2(static_cast<float>(frame_padding), static_cast<float>(frame_padding)) : style.FramePadding;
	const ImRect bb(pos, pos + size + padding*2);
	const ImRect inner_bb(pos + padding, pos + padding + size);

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, &id))
		return;

	// Render
	const ImU32 col = ImGui::GetColorU32(ImGuiCol_Button);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	if (bg_col.w > 0.0f)
		draw_list->AddRectFilled(inner_bb.Min, inner_bb.Max, ImGui::GetColorU32(bg_col));
	const ImVec2 leftupper = inner_bb.Min;
	const ImVec2 rightlower = inner_bb.Max;
	const ImVec2 rightcenter((rightlower.x - leftupper.x) * 0.8 + leftupper.x, (rightlower.y + leftupper.y) / 2);
	const ImVec2 leftcenter((rightlower.x - leftupper.x) * 0.35 + leftupper.x, (rightlower.y + leftupper.y) / 2);
	const ImVec2 up(0, - abs(leftupper.y - rightlower.y) * 0.4);
	const ImVec2 left(-up.y, up.x);
	float thrust_fwd   = fmax( thrust.z, 0);
	float thrust_bwd   = fmax(-thrust.z, 0);
	float thrust_left  = fmax(-thrust.x, 0);
	float thrust_right = fmax( thrust.x, 0);
	float thrust_up    = fmax(-thrust.y, 0);
	float thrust_down  = fmax( thrust.y, 0);
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

bool PiGui::LowThrustButton(const char* id_string, const ImVec2& size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg)
{
	std::string label = std::to_string(thrust_level);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(id_string);
	const ImVec2 label_size = ImGui::CalcTextSize(label.c_str(), NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	// if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
	//     pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2(static_cast<float>(frame_padding), static_cast<float>(frame_padding)) : style.FramePadding;
	const ImRect bb(pos, pos + size + padding*2);
	const ImRect inner_bb(pos + padding, pos + padding + size);

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, &id))
		return false;

	// if (window->DC.ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0); // flags

	// Render
	const ImU32 col = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	const ImVec2 center = (inner_bb.Min + inner_bb.Max) / 2;
	float radius = (inner_bb.Max.x - inner_bb.Min.x) * 0.4;
	float thickness = 4;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	if (bg_col.w > 0.0f)
		draw_list->AddRectFilled(inner_bb.Min, inner_bb.Max, ImGui::GetColorU32(bg_col));

	draw_list->PathArcTo(center, radius, 0, IM_PI * 2, 16);
	draw_list->PathStroke(gauge_bg, false, thickness);

	draw_list->PathArcTo(center, radius, IM_PI, IM_PI + IM_PI * 2 * (thrust_level / 100.0) , 16);
	draw_list->PathStroke(gauge_fg, false, thickness);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label.c_str(), NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	return pressed;
}

void PiGui::Cleanup() {
	for(auto tex : m_svg_textures) {
		delete tex;
	}
}

PiGui::PiGui() : m_should_bake_fonts(true) {
	PiFont uiheading("orbiteer",
									 { PiFace("DejaVuSans.ttf", /*18.0/20.0*/ 1.2, {{0x400, 0x4ff}, {0x500, 0x527}}),
											 PiFace("wqy-microhei.ttc", 1.0, {{0x4e00, 0x9fff},{0x3400,0x4dff}}),
											 PiFace("Orbiteer-Bold.ttf", 1.0, {{0, 0xffff}}) // imgui only supports 0xffff, not 0x10ffff
											 });
	PiFont guifont("pionillium",
								 {
									 PiFace("DejaVuSans.ttf", 13.0/14.0, {{0x400, 0x4ff}, {0x500, 0x527}}),
										 PiFace("wqy-microhei.ttc", 1.0, {{0x4e00, 0x9fff},{0x3400,0x4dff}}),
										 PiFace("PionilliumText22L-Medium.ttf", 1.0, {{0, 0xffff}})
										 });
	AddFontDefinition(uiheading);
	AddFontDefinition(guifont);

	Output("Fonts:\n");
	for(auto entry : m_font_definitions) {
		//		Output("  entry %s:\n", entry.first.c_str());
		entry.second.describe();
	}

	// ensure the tooltip font exists
	GetFont("pionillium", 14);

};

const bool PiFace::containsGlyph(unsigned short glyph) const {
	for(auto range : m_ranges) {
		if(range.first <= glyph && glyph <= range.second)
			return true;
	}
	return false;
}

void PiFace::addGlyph(unsigned short glyph) {
	// Output("- PiFace %s adding glyph 0x%x\n", ttfname().c_str(), glyph);
	for(auto &range : m_used_ranges) {
		if(range.first <= glyph && glyph <= range.second) {
			// Output(" - already added, not adding again\n");
			return;
		}
	}
	//	Output(" - added\n");
	m_used_ranges.push_back(std::make_pair(glyph, glyph));
}

void PiFace::sortUsedRanges() const {
	// sort by ascending lower end of range
	std::sort(m_used_ranges.begin(), m_used_ranges.end(), [](const std::pair<unsigned short, unsigned short> &a, const std::pair<unsigned short, unsigned short> &b) { return a.first < b.first; });
	// merge adjacent ranges
	std::vector<std::pair<unsigned short, unsigned short>> merged;
	std::pair<unsigned short, unsigned short> current(0xffff,0xffff);
	for(auto &range : m_used_ranges) {
		//		Output("> checking 0x%x-0x%x\n", range.first, range.second);
		if(current.first == 0xffff && current.second == 0xffff)
			current = range;
		else {
			// if only a few are missing in range, just merge nontheless. +5 is 4 missing
			if(current.second + 5 >= range.first) { // (current.second + 1 == range.first)
				//				Output("> merging 0x%x-0x%x and 0x%x-0x%x\n", current.first, current.second, range.first, range.second);
				current.second = range.second;
			} else {
				//				Output("> pushing 0x%x-0x%x\n", current.first, current.second);
				merged.push_back(current);
				current = range;
			}
		}
	}
	if(current.first != 0xffff && current.second != 0xffff)
		merged.push_back(current);
	m_used_ranges.assign(merged.begin(), merged.end());
}
