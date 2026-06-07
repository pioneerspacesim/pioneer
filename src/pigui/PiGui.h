// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

// #include "FileSystem.h"
#include "RefCounted.h"
#include "imgui/imgui.h"

// #include "utils.h"

#include <memory>
#include <map>
#include <vector>

union SDL_Event;

namespace Graphics {
	class Texture;
	class Renderer;
} // namespace Graphics

class GuiApplication;

namespace PiGui {

	struct PiSVGLoader;
	struct SVGFontFile;
	struct SVGFontBaked;

	class RasterizeSVGTask;
	struct RasterizeSVGResult;

	using FontPair = std::pair<ImFontConfig *, ImFontBaked *>;

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

		ImFont *GetFont(const std::string &name);

		bool ProcessEvent(SDL_Event *event);

	private:
		friend struct PiSVGLoader;

		GuiApplication *m_app;
		Graphics::Renderer *m_renderer;
		std::unique_ptr<InstanceRenderer> m_instanceRenderer;

		// Stores the pointer to the ini file name given to ImGui,
		// so we can delete[] the memory again when uninitializing.
		char *m_ioIniFilename;

		std::map<std::string, SVGFontFile> m_svgSources;

		std::map<FontPair, std::vector<ImWchar>> m_pendingGlyphs;
		std::map<std::string, std::vector<SVGFontBaked>> m_bakedSvgFonts;

		std::map<std::string, ImFont *> m_fontMap;

		std::vector<RasterizeSVGTask *> m_svgFontTasks;
		std::vector<std::unique_ptr<uint8_t[]>> m_svgRasterData;

		ImGuiStyle m_debugStyle;
		bool m_debugStyleActive;

		void LoadFontDefinitionFromFile(const std::string &filePath);

		void RequestSVGFaceData(SVGFontBaked *request);
		void CancelSVGFaceData(FontPair for_font);

		SVGFontBaked *GetBakedFont(const std::string &filename, uint32_t pixel_size);
	};

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
