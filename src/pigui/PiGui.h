// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "FileSystem.h"
#include "RefCounted.h"
#include "imgui/imgui.h"

#include "utils.h"

#include <map>
#include <unordered_set>

union SDL_Event;

namespace Graphics {
	class Texture;
	class Renderer;
} // namespace Graphics

class GuiApplication;

namespace PiGui {

	class RasterizeSVGTask;

	struct RasterizeSVGResult {
		RasterizeSVGResult(uint8_t *data, int width, int height) :
			data(data),
			width(width),
			height(height)
		{
		}

		std::unique_ptr<uint8_t[]> data;
		int width;
		int height;
	};

	class PiFace {
	public:
		using UsedRange = std::pair<uint16_t, uint16_t>;

		PiFace(const std::string &ttfname, float sizefactor) :
			m_ttfname(ttfname),
			m_sizefactor(sizefactor) {}

		PiFace(const std::string &svgname, uint16_t startGlyph, int columns, int rows) :
			m_svgname(svgname),
			m_svgrows(rows),
			m_svgcolumns(columns),
			m_loadrange({ startGlyph, startGlyph + (rows * columns) })
		{
		}

		const std::string &ttfname() const { return m_ttfname; }
		float sizefactor() const { return m_sizefactor; }

		const std::string &svgname() const { return m_svgname; }
		bool isSvgFont() const { return !m_svgname.empty(); }

		int svgRows() const { return m_svgrows; }
		int svgCols() const { return m_svgcolumns; }

		// Add this fontface at the specified size to the global font atlas
		ImFont *addTTFFaceToAtlas(int pixelSize, ImFontConfig *config, ImVector<ImWchar> *ranges);

		// Add this SVG fontface at the specified size to the global font atlas
		ImFont *addSVGFaceToAtlas(int pixelSize, ImFontConfig *config, ImVector<ImWchar> *ranges, RasterizeSVGResult *svgData, ImVector<int> *outGlyphRects);
		// Copy the pixel data for this fontface into the global font atlas
		void finishSVGFaceData(ImFont *font, int pixelSize, RasterizeSVGResult *svgData, ImVector<int> *glyphRects);

	private:
		friend class Instance; // need access to some private data

		std::string m_ttfname; // only the ttf name, it is automatically sought in data/fonts/
		float m_sizefactor;	   // the requested pixelsize is multiplied by this factor

		std::string m_svgname; // path to svg file for font-face
		int m_svgrows;		   // number of character rows
		int m_svgcolumns;	   // number of character columns
		UsedRange m_loadrange; // start and end of font glyphs to load
	};

	struct PiFontDefinition {
	public:
		PiFontDefinition(const std::string &name) :
			name(name) {}
		PiFontDefinition(const std::string &name, const std::vector<PiFace> &faces) :
			name(name),
			faces(faces) {}
		PiFontDefinition() :
			name("unknown") {}

		std::string name;
		std::vector<PiFace> faces;
		bool loadDefaultRange = true;
	};

	class PiFont {
	public:
		using UsedRange = std::pair<uint16_t, uint16_t>;

		struct CustomGlyphData {
			ImFont *font;
			PiFace *face;
			std::unique_ptr<ImVector<int>> glyphRects;
			RasterizeSVGResult *svgData;
		};

		PiFont(PiFontDefinition &fontDef, int pixelSize) :
			m_fontDef(fontDef),
			m_pixelsize(pixelSize)
		{
		}

		const std::string &name() const { return m_fontDef.name; }
		std::vector<PiFace> &faces() const { return m_fontDef.faces; }
		const std::vector<UsedRange> &used_ranges() const { return m_used_ranges; }
		int pixelsize() const { return m_pixelsize; }
		const PiFontDefinition &definition() const { return m_fontDef; }

		std::vector<CustomGlyphData> &custom_glyphs() { return m_custom_glyphs; }

		void describe(bool withFaces = false) const;

		bool addGlyph(unsigned short glyph);

	private:
		PiFontDefinition &m_fontDef;
		int m_pixelsize;
		std::vector<UsedRange> m_used_ranges;
		std::vector<CustomGlyphData> m_custom_glyphs;
	};

	class InstanceRenderer;

	/* Class to wrap ImGui. */
	class Instance : public RefCounted {
	public:
		Instance(GuiApplication *app);

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

	private:
		GuiApplication *m_app;
		Graphics::Renderer *m_renderer;
		std::unique_ptr<InstanceRenderer> m_instanceRenderer;

		// Stores the pointer to the ini file name given to ImGui,
		// so we can delete[] the memory again when uninitializing.
		char *m_ioIniFilename;

		std::map<std::pair<std::string, int>, ImFont *> m_fonts;
		std::map<ImFont *, std::pair<std::string, int>> m_im_fonts;
		std::map<std::pair<std::string, int>, PiFont> m_pi_fonts;
		bool m_should_bake_fonts;

		std::map<std::string, PiFontDefinition> m_font_definitions;

		std::vector<ImVector<ImWchar> *> m_glyphRanges;
		std::vector<RasterizeSVGTask *> m_svgFontTasks;
		std::map<std::string, std::vector<RasterizeSVGResult>> m_svgFontRasterData;

		ImGuiStyle m_debugStyle;
		bool m_debugStyleActive;

		void AddFontDefinition(const PiFontDefinition &font)
		{
			m_font_definitions[font.name] = font;
		}

		void LoadFontDefinitionFromFile(const std::string &filePath);

		void BakeFonts();
		void BakeFont(PiFont &font);
		void ClearFonts();

		RasterizeSVGResult *RequestSVGFaceData(PiFace *face, int pixelsize);
	};

	int RadialPopupSelectMenu(const ImVec2 center, const char *popup_id, int mouse_button, const std::vector<ImTextureID> &tex_ids, const std::vector<std::pair<ImVec2, ImVec2>> &uvs, const std::vector<ImU32> &colors, const std::vector<const char *> &tooltips, unsigned int size, unsigned int padding);
	bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	bool LowThrustButton(const char *label, const ImVec2 &size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg);
	bool ButtonImageSized(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &imgSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

	void ThrustIndicator(const std::string &id_string, const ImVec2 &size, const ImVec4 &thrust, const ImVec4 &velocity, const ImVec4 &bg_col, int frame_padding, ImColor vel_fg, ImColor vel_bg, ImColor thrust_fg, ImColor thrust_bg);

	// we need to know if the change was made by direct input or the change was
	// made by mouse movement, to successfully serve values such as YYYY-MM-DD
	// when changing with the mouse, we get some internal delta value and add
	// it to previous internal value
	// when we receive direct input from the keyboard, we get a completely
	// different (face value), an integer of the form YYYY-MM-DD so we just
	// convert it to internal value.
	// Also see: data/pigui/modules/new-game-window/location.lua
	enum class DragChangeMode : unsigned { NOT_CHANGED, CHANGED, CHANGED_BY_TYPING };
	DragChangeMode IncrementDrag(const char *label, double &v, float v_speed, const double v_min, const double v_max, const char *format, bool draw_progress_bar);

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
