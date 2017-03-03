// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RefCounted.h"
#include "graphics/opengl/RendererGL.h"
#include "Lua.h"
#include "LuaRef.h"
#include "LuaTable.h"
#include "FileSystem.h"
#include "imgui/imgui.h"
#include "imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h"
#include "imgui/examples/sdl_opengl2_example/imgui_impl_sdl.h"

/* Class to wrap ImGui. */
class PiGui : public RefCounted {
public:
	static ImFont *pionillium12;
	static ImFont *pionillium15;
	static ImFont *pionillium18;
	static ImFont *pionillium30;
	static ImFont *pionillium36;
	static ImFont *orbiteer18;
	static ImFont *orbiteer30;

	PiGui() {}

	LuaRef GetHandlers() const { return m_handlers; }

	LuaRef GetKeys() const { return m_keys; }

	void Render(double delta, std::string handler = "GAME");

	void Init(SDL_Window *window);

	void Uninit() {
		Cleanup();
		m_handlers.Unref();
		m_keys.Unref();
	}

	static ImTextureID RenderSVG(std::string svgFilename, int width, int height);

	static void NewFrame(SDL_Window *window);

	static void RenderImGui() { ImGui::Render(); }

	static bool ProcessEvent(SDL_Event *event);

	static void *makeTexture(const std::string &filename, unsigned char *pixels, int width, int height);

	static bool WantCaptureMouse() {
		return ImGui::GetIO().WantCaptureMouse;
	}

	static bool WantCaptureKeyboard() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}
	static int RadialPopupSelectMenu(const ImVec2& center, std::string popup_id, std::vector<ImTextureID> tex_ids, std::vector<std::pair<ImVec2,ImVec2>> uvs, unsigned int size, std::vector<std::string> tooltips);
	static bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	void Cleanup();
private:
	LuaRef m_handlers;
	LuaRef m_keys;

	static std::vector<std::pair<std::string,Graphics::Texture*>> m_svg_textures;
};
