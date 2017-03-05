// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "graphics/opengl/TextureGL.h" // nasty, usage of GL is implementation specific
#include "PiGui.h"
#include "imgui/imgui_internal.h"

#include <stdio.h>
#include <string.h>
#include <float.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"

std::vector<std::pair<std::string,Graphics::Texture*>> PiGui::m_svg_textures;

ImFont *PiGui::pionillium12 = nullptr;
ImFont *PiGui::pionillium15 = nullptr;
ImFont *PiGui::pionillium18 = nullptr;
ImFont *PiGui::pionillium30 = nullptr;
ImFont *PiGui::pionillium36 = nullptr;
ImFont *PiGui::orbiteer18 = nullptr;
ImFont *PiGui::orbiteer30 = nullptr;

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

	// re-use existing texture if already loaded
	for(auto strTex : m_svg_textures) {
		if(strTex.first == svgFilename) {
			// nasty bit as I invoke the TextureGL
			Graphics::OGL::TextureGL *pGLTex = reinterpret_cast<Graphics::OGL::TextureGL*>(strTex.second);
			Uint32 result = pGLTex->GetTextureID();
 			Output("Re-used existing texture with id: %i\n", result);
			return reinterpret_cast<void*>(result);
		}
	}

	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w, h;
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
	h = static_cast<int>(image->height);

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
	return makeTexture(svgFilename, img, W, H);
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

	static unsigned short glyph_ranges[] = { 0x1, 0x3c0, 0x0, 0x0 };
	pionillium12 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "PionilliumText22L-Medium.ttf").c_str(), 12.0f, nullptr, glyph_ranges);
	pionillium15 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "PionilliumText22L-Medium.ttf").c_str(), 15.0f, nullptr, glyph_ranges);
	pionillium18 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "PionilliumText22L-Medium.ttf").c_str(), 18.0f, nullptr, glyph_ranges);
	pionillium30 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "PionilliumText22L-Medium.ttf").c_str(), 30.0f, nullptr, glyph_ranges);
	pionillium36 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "PionilliumText22L-Medium.ttf").c_str(), 36.0f, nullptr, glyph_ranges);
	orbiteer18 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "Orbiteer-Bold.ttf").c_str(), 18.0f, nullptr, glyph_ranges);
	orbiteer30 = io.Fonts->AddFontFromFileTTF(FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), "Orbiteer-Bold.ttf").c_str(), 30.0f, nullptr, glyph_ranges);
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

void *PiGui::makeTexture(const std::string &filename, unsigned char *pixels, int width, int height)
{
	// this is not very pretty code and uses the Graphics::TextureGL class directly
	// Texture descriptor defines the size, type.
	// Gone for LINEAR_CLAMP here and RGBA like the original code
	const vector2f texSize(1.0f, 1.0f);
	const vector2f dataSize(width, height);
	const Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888,
		dataSize, texSize, Graphics::LINEAR_CLAMP,
		false, false, false, 0, Graphics::TEXTURE_2D);
	// Create the texture, calling it via renderer directly avoids the caching call of TextureBuilder
	// However interestingly this gets called twice which would have been a WIN for the TextureBuilder :/
	Graphics::Texture *pTex = Pi::renderer->CreateTexture(texDesc);
	// Update it with the actual pixels, this is a two step process due to legacy code
	pTex->Update(pixels, dataSize, Graphics::TEXTURE_RGBA_8888);
	Pi::renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
	// nasty bit as I invoke the TextureGL
	Graphics::OGL::TextureGL *pGLTex = reinterpret_cast<Graphics::OGL::TextureGL*>(pTex);
	Uint32 result = pGLTex->GetTextureID();
 	Output("texture id: %i\n", result);
	m_svg_textures.push_back( std::make_pair(filename,pTex) );	// store for cleanup later
 	return reinterpret_cast<void*>(result);
}

void PiGui::NewFrame(SDL_Window *window) {
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
	if(Pi::DoingMouseGrab())
	{
		ImGui::GetIO().MouseDrawCursor = false;
	}
	else
	{
		ImGui::GetIO().MouseDrawCursor = true;
	}
}

void PiGui::Cleanup() {
	for(auto strTex : m_svg_textures) {
		delete strTex.second;
	}
}

void PiGui::Render(double delta, std::string handler) {
	ScopedTable t(m_handlers);
	if(t.Get<bool>(handler)) {
		t.Call<bool>(handler, delta);
		Pi::renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
	}
}
