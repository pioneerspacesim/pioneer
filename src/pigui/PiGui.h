// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "FileSystem.h"
#include "RefCounted.h"
#include "imgui/imgui.h"

#include "utils.h"

#include <unordered_set>

namespace Graphics {
	class Texture;
	class Renderer;
} // namespace Graphics

namespace PiGui {

	class PiFace {
	public:
		using UsedRange = std::pair<uint16_t, uint16_t>;
		PiFace(const std::string &ttfname, float sizefactor) :
			m_ttfname(ttfname),
			m_sizefactor(sizefactor) {}

		const std::string &ttfname() const { return m_ttfname; }

		float sizefactor() const { return m_sizefactor; }

		//std::unordered_map<unsigned short, unsigned short> &invalid_glyphs() const { return m_invalid_glyphs; }
		const std::vector<UsedRange> &used_ranges() const { return m_used_ranges; }

		bool isValidGlyph(unsigned short glyph) const;
		void addGlyph(unsigned short glyph);
		void sortUsedRanges() const;

	private:
		friend class Instance; // need access to some private data

		std::string m_ttfname; // only the ttf name, it is automatically sought in data/fonts/
		float m_sizefactor;	   // the requested pixelsize is multiplied by this factor

		std::unordered_set<unsigned short> m_invalid_glyphs;
		mutable std::vector<UsedRange> m_used_ranges;

		ImVector<ImWchar> m_imgui_ranges;
	};

	class PiFont {
	public:
		PiFont(const std::string &name) :
			m_name(name) {}
		PiFont(const std::string &name, const std::vector<PiFace> &faces) :
			m_name(name),
			m_faces(faces) {}
		PiFont() :
			m_name("unknown") {}

		const std::vector<PiFace> &faces() const { return m_faces; }
		std::vector<PiFace> &faces() { return m_faces; }

		const std::string &name() const { return m_name; }

		int pixelsize() const { return m_pixelsize; }
		void setPixelsize(int pixelsize) { m_pixelsize = pixelsize; }

		void describe() const
		{
			Output("font %s:\n", name().c_str());
			for (const PiFace &face : faces()) {
				Output("- %s %f\n", face.ttfname().c_str(), face.sizefactor());
			}
		}

	private:
		std::string m_name;
		std::vector<PiFace> m_faces;
		int m_pixelsize;
	};

	class InstanceRenderer;

	/* Class to wrap ImGui. */
	class Instance : public RefCounted {
	public:
		Instance();

		void Init(Graphics::Renderer *renderer);
		void Uninit();

		InstanceRenderer *GetRenderer() { return m_instanceRenderer.get(); }

		// Call at the start of every frame. Calls ImGui::NewFrame() internally.
		void NewFrame();

		// Call at the end of a frame that you're not going to render the results of
		void EndFrame();

		// Calls ImGui::EndFrame() internally and does book-keeping before rendering.
		void Render();

		// Sets the ImGui Style object to use the predefined development tooling style
		void SetDebugStyle();

		// Sets the ImGui Style object to use the game UI style object as modified by Lua
		void SetNormalStyle();

		ImFont *AddFont(const std::string &name, int size);
		ImFont *GetFont(const std::string &name, int size);

		void AddGlyph(ImFont *font, unsigned short glyph);

		bool ProcessEvent(SDL_Event *event);

		void RefreshFontsTexture();

	private:
		Graphics::Renderer *m_renderer;
		std::unique_ptr<InstanceRenderer> m_instanceRenderer;

		// Stores the pointer to the ini file name given to ImGui,
		// so we can delete[] the memory again when uninitializing.
		char *m_ioIniFilename;

		std::map<std::pair<std::string, int>, ImFont *> m_fonts;
		std::map<ImFont *, std::pair<std::string, int>> m_im_fonts;
		std::map<std::pair<std::string, int>, PiFont> m_pi_fonts;
		bool m_should_bake_fonts;

		std::map<std::string, PiFont> m_font_definitions;

		ImGuiStyle m_debugStyle;
		bool m_debugStyleActive;

		void BakeFonts();
		void BakeFont(PiFont &font);
		void AddFontDefinition(const PiFont &font) { m_font_definitions[font.name()] = font; }
		void ClearFonts();
	};

	int RadialPopupSelectMenu(const ImVec2 center, const char *popup_id, int mouse_button, const std::vector<ImTextureID> &tex_ids, const std::vector<std::pair<ImVec2, ImVec2>> &uvs, const std::vector<ImU32> &colors, const std::vector<const char *> &tooltips, unsigned int size, unsigned int padding);
	bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	bool LowThrustButton(const char *label, const ImVec2 &size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg);
	bool ButtonImageSized(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &imgSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

	void ThrustIndicator(const std::string &id_string, const ImVec2 &size, const ImVec4 &thrust, const ImVec4 &velocity, const ImVec4 &bg_col, int frame_padding, ImColor vel_fg, ImColor vel_bg, ImColor thrust_fg, ImColor thrust_bg);

	void IncrementDrag(const std::string &label, int &v, const int v_min, const int v_max, const std::string &format);

	inline bool WantCaptureMouse()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	inline bool WantCaptureKeyboard()
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	std::vector<Graphics::Texture *> &GetSVGTextures();
	ImTextureID RenderSVG(Graphics::Renderer *renderer, std::string svgFilename, int width, int height);

} //namespace PiGui
