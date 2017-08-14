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

class PiFace {
	std::string m_ttfname; // only the ttf name, it is automatically sought in data/fonts/
	float m_sizefactor; // the requested pixelsize is multiplied by this factor
	std::vector<std::pair<unsigned short, unsigned short>> m_ranges;
	mutable std::vector<std::pair<unsigned short, unsigned short>> m_used_ranges;
public:
	PiFace(const std::string &ttfname, float sizefactor) : m_ttfname(ttfname), m_sizefactor(sizefactor) {}
	PiFace(const std::string &ttfname, float sizefactor, const std::vector<std::pair<unsigned short, unsigned short>> &ranges) : m_ttfname(ttfname), m_sizefactor(sizefactor), m_ranges(ranges) {}
	const std::string &ttfname() const { return m_ttfname; }
	const float sizefactor() const { return m_sizefactor; }
	const std::vector<std::pair<unsigned short, unsigned short>> &ranges() const { return m_ranges; }
	const std::vector<std::pair<unsigned short, unsigned short>> &used_ranges() const { return m_used_ranges; }
	const bool containsGlyph(unsigned short glyph) const;
	void addGlyph(unsigned short glyph);
	void sortUsedRanges() const;
};

class PiFont {
	std::string m_name;
	std::vector<PiFace> m_faces;
	int m_pixelsize;
public:
	PiFont(const std::string &name) : m_name(name) {}
	PiFont(const std::string &name, const std::vector<PiFace> &faces) : m_name(name), m_faces(faces) {}
	PiFont(const PiFont &other) : m_name(other.name()), m_faces(other.faces()) {}
	PiFont() : m_name("unknown") {}
	const std::vector<PiFace> &faces() const { return m_faces; }
	std::vector<PiFace> &faces() { return m_faces; }
	const std::string &name() const { return m_name; }
	int pixelsize() const { return m_pixelsize; }
	void setPixelsize(int pixelsize) { m_pixelsize = pixelsize; }
	void describe() const {
		Output("font %s:\n", name().c_str());
		for(const PiFace &face : faces()) {
			Output("- %s %f\n", face.ttfname().c_str(), face.sizefactor());
		}
	}
};


/* Class to wrap ImGui. */
class PiGui : public RefCounted {
	std::map<std::pair<std::string,int>, ImFont*> m_fonts;
	std::map<ImFont*, std::pair<std::string,int>> m_im_fonts;
	std::map<std::pair<std::string,int>, PiFont> m_pi_fonts;
	bool m_should_bake_fonts;

	std::map<std::string,PiFont> m_font_definitions;

	void AddGlyph(ImFont *font, unsigned short glyph);
	void BakeFonts();
	void BakeFont(const PiFont &font);
	void AddFontDefinition(const PiFont &font) { m_font_definitions[font.name()] = font; }
	void ClearFonts();
public:

	PiGui();

	LuaRef GetHandlers() const { return m_handlers; }

	LuaRef GetKeys() const { return m_keys; }

	void Render(double delta, std::string handler = "GAME");

	void Init(SDL_Window *window);

	ImFont *GetFont(const std::string &name, int size);

	void Uninit() {
		Cleanup();
		m_handlers.Unref();
		m_keys.Unref();
	}
	ImFont *AddFont(const std::string &name, int size);

	static ImTextureID RenderSVG(std::string svgFilename, int width, int height);

	static void NewFrame(SDL_Window *window);

	static void EndFrame();

	static void RenderImGui() { ImGui::Render(); }

	static bool ProcessEvent(SDL_Event *event);

	void RefreshFontsTexture();

	static void *makeTexture(unsigned char *pixels, int width, int height);

	static bool WantCaptureMouse() {
		return ImGui::GetIO().WantCaptureMouse;
	}

	static bool WantCaptureKeyboard() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}
	static int RadialPopupSelectMenu(const ImVec2& center, std::string popup_id, std::vector<ImTextureID> tex_ids, std::vector<std::pair<ImVec2,ImVec2>> uvs, unsigned int size, std::vector<std::string> tooltips);
	static bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	void Cleanup();
	static bool LowThrustButton(const char* label, const ImVec2& size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg);

	static void ThrustIndicator(const std::string &id_string, const ImVec2& size, const ImVec4& thrust, const ImVec4& velocity, const ImVec4 &bg_col, int frame_padding, ImColor vel_fg, ImColor vel_bg, ImColor thrust_fg, ImColor thrust_bg);
private:
	LuaRef m_handlers;
	LuaRef m_keys;
	static std::vector<Graphics::Texture*> m_svg_textures;
};
