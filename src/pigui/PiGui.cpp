// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGui.h"
#include "FileSystem.h"
#include "Input.h"
#include "JsonUtils.h"
#include "Pi.h"
#include "PiGuiRenderer.h"

#include "core/TaskGraph.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"

#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"

using namespace PiGui;

namespace {
	std::vector<Graphics::Texture *> m_svg_textures;
}

std::vector<Graphics::Texture *> &PiGui::GetSVGTextures()
{
	return m_svg_textures;
}

// Handle GPU upload of texture image data on the main application thread.
class UpdateImageTask : public Task {
public:
	Graphics::Texture *texture;
	const unsigned char *imageData;

	UpdateImageTask(Graphics::Texture *tex, const unsigned char *data) :
		texture(tex),
		imageData(data)
	{
	}

	virtual void OnExecute(TaskRange range) override
	{
		PROFILE_SCOPED()

		const Graphics::TextureDescriptor &desc = texture->GetDescriptor();
		texture->Update(imageData, desc.dataSize, desc.format);
		delete[] imageData;
	}
};

// Run SVG loading and rasterization on a separate thread, defer GPU upload until end-of-frame.
class PiGui::RasterizeSVGTask : public Task, public CompleteNotifier {
public:
	// Rasterize an SVG file to a texture and upload to GPU on main thread
	RasterizeSVGTask(std::string filename, int width, int height, Graphics::Texture *outputTexture) :
		filename(filename),
		width(width),
		height(height),
		texture(outputTexture)
	{
	}

	// Rasterize an SVG file to CPU buffer for use with font data
	RasterizeSVGTask(std::string filename, int width, int height, PiFace *fontFace) :
		filename(filename),
		width(width),
		height(height),
		texture(nullptr),
		fontFace(fontFace)
	{
		SetOwner(this);
	}

	bool LoadFile()
	{
		PROFILE_SCOPED();

		image = nsvgParseFromFile(filename.c_str(), "px", 96.0f);
		if (image == NULL) {
			Log::Error("Could not open SVG image {}.\n", filename);
			return false;
		}

		return true;
	}

	virtual void OnExecute(TaskRange range) override
	{
		PROFILE_SCOPED()

		if (!LoadFile())
			return;

		size_t stride = width * 4;
		imageData = new uint8_t[stride * height];

		if (!imageData) {
			Log::Error("Couldn't allocate memory for SVG image {}.\n", filename);
			return;
		}

		memset(imageData, 0, stride * height);

		NSVGrasterizer *rast = nsvgCreateRasterizer();
		if (!rast) {
			Log::Error("Couldn't create SVG rasterizer for SVG image {}.\n", filename);
			delete[] imageData;
			return;
		}

		float scale = double(width) / int(image->width);
		nsvgRasterize(rast, image, 0, 0, scale, imageData, width, height, stride);

		nsvgDeleteRasterizer(rast);
		nsvgDelete(image);

		if (texture) {
			Pi::GetApp()->GetTaskGraph()->QueueTaskPinned(new UpdateImageTask(texture, imageData));
			imageData = nullptr;
		}
	}

	virtual void OnComplete() override
	{
		if (imageData)
			delete[] imageData;
	}

	uint8_t *GetImageData() { return imageData; }
	PiFace *GetFontFace() { return fontFace; }

public:
	std::string filename;
	int width;
	int height;

private:
	Graphics::Texture *texture;
	uint8_t *imageData;
	PiFace *fontFace;
	NSVGimage *image;
};

static Graphics::Texture *makeSVGTexture(Graphics::Renderer *renderer, int width, int height)
{
	const vector3f dataSize(width, height, 0.f);
	const Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888, dataSize,
		Graphics::LINEAR_CLAMP, false, false, false, 0,
		Graphics::TEXTURE_2D);

	Graphics::Texture *tex = renderer->CreateTexture(texDesc);
	return tex;
}

ImTextureID PiGui::RenderSVG(Graphics::Renderer *renderer, std::string svgFilename, int width, int height)
{
	PROFILE_SCOPED();

	Graphics::Texture *tex = makeSVGTexture(renderer, width, height);
	PiGui::GetSVGTextures().push_back(tex);

	Pi::GetApp()->GetTaskGraph()->QueueTask(new RasterizeSVGTask(svgFilename, width, height, tex));

	return ImTextureID(tex);
}

// Colors taken with love from the Limit Theory editor
// http://forums.ltheory.com/viewtopic.php?f=30&t=6459
void StyleColorsDarkPlus(ImGuiStyle &style)
{
	style.FramePadding = ImVec2(6, 5);

	style.WindowRounding = 5.0;
	style.ChildRounding = 2.0;
	style.FrameRounding = 2.0;
	style.GrabRounding = 2.0;
	style.TabRounding = 2.0;

	style.FrameBorderSize = 1.0;
	style.TabBorderSize = 1.0;

	style.Colors[ImGuiCol_WindowBg] = ImColor(24, 26, 31);
	style.Colors[ImGuiCol_ChildBg] = ImColor(20, 22, 26);
	style.Colors[ImGuiCol_PopupBg] = ImColor(20, 22, 26, 240);
	style.Colors[ImGuiCol_Border] = ImColor(0, 0, 0);

	style.Colors[ImGuiCol_FrameBg] = ImColor(33, 36, 43);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(45, 50, 59);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(56, 126, 210);

	style.Colors[ImGuiCol_TitleBg] = ImColor(20, 23, 26);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(27, 31, 35);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(15, 17, 19);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(20, 23, 26);

	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(19, 20, 24);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(33, 36, 43);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(81, 88, 105);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(100, 109, 130);

	style.Colors[ImGuiCol_Button] = ImColor(51, 56, 67);
	style.Colors[ImGuiCol_Header] = ImColor(51, 56, 67);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(56, 126, 210);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(66, 150, 250);

	style.Colors[ImGuiCol_Tab] = ImColor(20, 23, 26);
	style.Colors[ImGuiCol_TabActive] = ImColor(60, 133, 224);
	style.Colors[ImGuiCol_TabHovered] = ImColor(66, 150, 250);
}

//
//	PiGui::Instance
//

Instance::Instance(GuiApplication *app) :
	m_app(app),
	m_should_bake_fonts(true),
	m_debugStyle(),
	m_debugStyleActive(false)
{
	FileSystem::FileEnumerator dir(FileSystem::gameDataFiles, "fonts/");
	for(const FileSystem::FileInfo &fileInfo : dir) {
		if (ends_with_ci(fileInfo.GetPath(), ".json")) {
			try {
				LoadFontDefinitionFromFile(fileInfo.GetPath());
			} catch (Json::type_error &e) {
				Log::Warning("Malformed font definition file {}, not loading.", fileInfo.GetPath());
			}
		}
	}

	Log::Info("Loaded PiGui fonts from disk:");
	for (auto &entry : m_font_definitions) {
		PiFont(entry.second, 0).describe(true);
	}

	// ensure the tooltip font exists
	GetFont("pionillium", 14);

	StyleColorsDarkPlus(m_debugStyle);
}

void Instance::LoadFontDefinitionFromFile(const std::string &filePath)
{
	Json fontInfo = JsonUtils::LoadJsonDataFile(filePath);

	std::string fontName = fontInfo["name"].get<std::string>();
	if (fontName.empty()) {
		Log::Warning("Malformed font definition {}, not loading.", filePath);
		return;
	}

	PiFontDefinition fontDef (fontName);
	fontDef.loadDefaultRange = fontInfo.value("loadDefaultRange", true);

	for (auto &entry : fontInfo["faces"]) {
		if (!entry.is_object())
			continue;

		if (entry["fontFile"].is_string()) {
			fontDef.faces.emplace_back(
				entry["fontFile"].get<std::string>(),
				entry.value("scale", 1.0)
			);
		} else if (entry["svgFile"].is_string()) {
			uint32_t rangeBase = 0xF000;
			sscanf(entry.value("rangeBase", "0xF000").c_str(), "%x", &rangeBase);

			fontDef.faces.emplace_back(
				entry["svgFile"].get<std::string>(),
				rangeBase,
				entry["grid"][0].get<int>(),
				entry["grid"][1].get<int>()
			);
		}
	}

	if (fontDef.faces.empty()) {
		Log::Warning("Font definition {} has no valid faces.", filePath);
		return;
	}

	AddFontDefinition(fontDef);
}

void Instance::SetDebugStyle()
{
	if (!m_debugStyleActive)
		std::swap(m_debugStyle, ImGui::GetStyle());

	m_debugStyleActive = true;
}

void Instance::SetNormalStyle()
{
	if (m_debugStyleActive)
		std::swap(m_debugStyle, ImGui::GetStyle());

	m_debugStyleActive = false;
}

ImFont *Instance::GetFont(const std::string &name, int size)
{
	PROFILE_SCOPED()
	auto iter = m_fonts.find(std::make_pair(name, size));
	if (iter != m_fonts.end())
		return iter->second;
	//	Output("GetFont: adding font %s at %i on demand\n", name.c_str(), size);
	ImFont *font = AddFont(name, size);

	return font;
}

// If after rendering, the dear ImGui lacks a glyph, this function is called
void Instance::AddGlyph(ImFont *font, unsigned short glyph)
{
	PROFILE_SCOPED()
	// range glyph..glyph
	auto iter = m_im_fonts.find(font);
	if (iter == m_im_fonts.end()) {
		Error("Cannot find font instance for ImFont %p\n", (void *)font);
		assert(false);
	}
	auto pifont_iter = m_pi_fonts.find(iter->second);
	if (pifont_iter == m_pi_fonts.end()) {
		Error("No registered PiFont for name %s size %i\n", iter->second.first.c_str(), iter->second.second);
		assert(false);
	}

	// add the glyph..glyph range in this font
	// the first face with a valid glyph will be used to represent it

	PiFont &pifont = pifont_iter->second;
	// rebake fonts if the font is capable of providing the glyph
	// ( see Instance::BakeFont )
	if (pifont.addGlyph(glyph))
		m_should_bake_fonts = true;
}

ImFont *Instance::AddFont(const std::string &name, int size)
{
	PROFILE_SCOPED()
	auto iter = m_font_definitions.find(name);
	if (iter == m_font_definitions.end()) {
		Error("No font definition with name %s\n", name.c_str());
		assert(false);
	}
	auto existing = m_fonts.find(std::make_pair(name, size));
	if (existing != m_fonts.end()) {
		Error("Font %s already exists at size %i\n", name.c_str(), size);
		assert(false);
	}

	PiFontDefinition &fontDef = iter->second;

	PiFont &font = m_pi_fonts.try_emplace(std::make_pair(name, size), fontDef, size).first->second;

	// here we add the range 0x0020 .. 0x0020 and 0xFFFD .. 0xFFFD to the font
	// so it can render at the very least the fallback character
	font.addGlyph(0x20);
	font.addGlyph(IM_UNICODE_CODEPOINT_INVALID);

	// Log::Info("adding font (from {}):", (void *)&iter->second);
	// font.describe();

	m_should_bake_fonts = true;

	// return nullptr, apparently
	return m_fonts[std::make_pair(name, size)];
}

// TODO: this isn't very RAII friendly, are we sure we need to call Init() seperately from creating the instance?
void Instance::Init(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	m_renderer = renderer;
	m_instanceRenderer.reset(new InstanceRenderer(renderer));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// TODO: FIXME before upgrading! The sdl_gl_context parameter is currently
	// unused, but that is slated to change very soon.
	// We will need to fill this with a valid pointer to the OpenGL context.
	ImGui_ImplSDL2_InitForOpenGL(m_renderer->GetSDLWindow(), NULL);
	switch (m_renderer->GetRendererType()) {
	default:
	case Graphics::RENDERER_DUMMY:
		Error("RENDERER_DUMMY is not a valid renderer, aborting.");
		return;
	case Graphics::RENDERER_OPENGL_3x:
		m_instanceRenderer->Initialize();
		break;
	}

	ImGuiIO &io = ImGui::GetIO();
	// Apply the base style
	ImGui::StyleColorsDark();

	// Disable ctrl+tab / ctrl+shift+tab window switching
	ImGui::SetShortcutRouting(ImGui::GetCurrentContext()->ConfigNavWindowingKeyNext, ImGuiKeyOwner_None);
	ImGui::SetShortcutRouting(ImGui::GetCurrentContext()->ConfigNavWindowingKeyPrev, ImGuiKeyOwner_None);

	std::string imguiIni = FileSystem::JoinPath(FileSystem::GetUserDir(), "imgui.ini");

	m_ioIniFilename = new char[imguiIni.size() + 1];
	std::strncpy(m_ioIniFilename, imguiIni.c_str(), imguiIni.size() + 1);
	io.IniFilename = m_ioIniFilename;
}

bool Instance::ProcessEvent(SDL_Event *event)
{
	PROFILE_SCOPED()
	ImGui_ImplSDL2_ProcessEvent(event);
	return false;
}

void Instance::NewFrame()
{
	PROFILE_SCOPED()

	// Iterate through our fonts and check if IMGUI wants a character we don't have.
	for (auto &iter : m_fonts) {
		ImFont *font = iter.second;
		// font might be nullptr, if it wasn't baked yet
		if (font && !font->MissingGlyphs.empty()) {
			//			Output("%s %i is missing glyphs.\n", iter.first.first.c_str(), iter.first.second);
			for (const auto &glyph : font->MissingGlyphs) {
				AddGlyph(font, glyph);
			}
			font->MissingGlyphs.clear();
		}
	}

	for (auto &task : m_svgFontTasks) {
		if (task->IsComplete()) {
			PiFace *face = task->GetFontFace();
			m_svgFontRasterData[face->svgname()].emplace_back(task->GetImageData(), task->width, task->height);

			delete task;
			task = nullptr;

			// we have improved SVG data for a font, rebuild the atlas
			m_should_bake_fonts = true;
		}
	}

	m_svgFontTasks.erase(std::remove(m_svgFontTasks.begin(), m_svgFontTasks.end(), nullptr), m_svgFontTasks.end());

	// Bake fonts before a frame is begun.
	// This avoids any dangling texture pointers from recreating the texture between
	// issuing draw commands and rendering
	if (m_should_bake_fonts) {
		BakeFonts();
	}

	switch (m_renderer->GetRendererType()) {
	default:
	case Graphics::RENDERER_DUMMY:
		Error("RENDERER_DUMMY is not a valid renderer, aborting.");
		return;
	case Graphics::RENDERER_OPENGL_3x:
		m_instanceRenderer->NewFrame();
		break;
	}
	ImGui_ImplSDL2_NewFrame(m_renderer->GetSDLWindow());
	ImGui::NewFrame();

	m_renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
	ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
}

void Instance::EndFrame()
{
	PROFILE_SCOPED()

	// Explicitly end frame, to show tooltips. Otherwise, they are shown at the next NextFrame,
	// which might crash because the font atlas was rebuilt, and the old fonts were cached inside imgui.
	ImGui::EndFrame();
}

void Instance::Render()
{
	PROFILE_SCOPED()
	EndFrame();

	// FIXME: renderer uses async command execution but imgui impl is still directly generating GL commands
	m_renderer->FlushCommandBuffers();

	ImGui::Render();

	switch (m_renderer->GetRendererType()) {
	default:
	case Graphics::RENDERER_DUMMY:
		return;
	case Graphics::RENDERER_OPENGL_3x:
		m_instanceRenderer->RenderDrawData(ImGui::GetDrawData());
		break;
	}
}

void Instance::ClearFonts()
{
	PROFILE_SCOPED()
	ImGuiIO &io = ImGui::GetIO();
	// TODO: should also release all glyph_ranges...
	m_fonts.clear();
	m_im_fonts.clear();
	io.Fonts->Clear();
}

RasterizeSVGResult *Instance::RequestSVGFaceData(PiFace *face, int pixelsize)
{
	int width = face->m_svgcolumns * pixelsize;
	int height = face->m_svgrows * pixelsize;

	RasterizeSVGResult *bestResult = nullptr;
	int bestWidth = 0;

	for (auto &result : m_svgFontRasterData[face->svgname()]) {
		if (std::abs(result.width - width) < std::abs(bestWidth - width)) {
			bestResult = &result;
			bestWidth = result.width;
		}
	}

	if (!bestResult || bestWidth != width) {
		std::string filename = FileSystem::JoinPathBelow(FileSystem::GetDataDir(), face->svgname());
		RasterizeSVGTask *rasterTask = new RasterizeSVGTask(filename, width, height, face);

		m_app->GetTaskGraph()->QueueTask(rasterTask);
		m_svgFontTasks.push_back(rasterTask);
	}

	return bestResult;
}

// this function rasterizes a specific font
void Instance::BakeFont(PiFont &font)
{
	PROFILE_SCOPED()
	ImGuiIO &io = ImGui::GetIO();
	ImFont *imfont = nullptr;

	// note that if there are no ranges at all in the font, it is ignored
	if (font.used_ranges().empty()) {
		Log::Warning("PiGui font {}:{} has no glyphs, not baking!", font.name(), font.pixelsize());
		return;
	}

	ImFontGlyphRangesBuilder gb;

	// ( default imgui glyph range - 0x0020 .. 0x00FF : Basic Latin + Latin Supplement )
	if (font.definition().loadDefaultRange)
		gb.AddRanges(io.Fonts->GetGlyphRangesDefault());

	// Add any glyphs outside of the default range that have been used at least once before
	ImWchar gr[3] = { 0, 0, 0 };
	for (const auto &range : font.used_ranges()) {
		gr[0] = range.first;
		gr[1] = range.second;
		gb.AddRanges(gr);
	}

	ImVector<ImWchar> *font_char_ranges = new ImVector<ImWchar>;
	m_glyphRanges.emplace_back(font_char_ranges);

	gb.BuildRanges(font_char_ranges);

	ImFontConfig config;

	// Set the ImGui font name for debugging purposes
	std::string name = fmt::format("{}:{}", font.name(), font.pixelsize());
	strncpy(config.Name, name.c_str(), 39);

	// The main face of the font should go first in the list, because:
	//
	// - when a glyph is loaded from the font, a search is started in
	// the faces, and the faces are scanned in the order of this list
	// ( see ImFontAtlasBuildWithStbTruetype in imgui.cpp )
	//
	// - the default imgui glyph range ( 0x20 .. 0xFF ) is almost always
	// defined in every font, so the first font will provide the glyphs for
	// the basic range
	//
	for (PiFace &face : font.faces()) {
		config.MergeMode = imfont != nullptr;

		if (face.isSvgFont()) {
			PiFont::CustomGlyphData data = {};
			data.face = &face;

			data.svgData = RequestSVGFaceData(&face, font.pixelsize());
			if (!data.svgData) {
				Log::Warning("No SVG data available to rasterize icon font {}", face.svgname());
				continue;
			}

			data.glyphRects.reset(new ImVector<int>);
			data.font = face.addSVGFaceToAtlas(font.pixelsize(), &config, font_char_ranges, data.svgData, data.glyphRects.get());

			if (!data.glyphRects->empty())
				font.custom_glyphs().emplace_back(std::move(data));
		} else {
			ImFont *f = face.addTTFFaceToAtlas(font.pixelsize(), &config, font_char_ranges);
			if (imfont != nullptr)
				assert(f == imfont);
			imfont = f;
		}
	}

	m_im_fonts[imfont] = std::make_pair(font.name(), font.pixelsize());
	// 	Output("setting %s %i to %p\n", font.name(), font.pixelsize(), imfont);
	m_fonts[std::make_pair(font.name(), font.pixelsize())] = imfont;
	if (!imfont->MissingGlyphs.empty()) {
		Log::Warning("PiGui: newly-built font {}:{} has glyphs missing", font.name(), font.pixelsize());
		imfont->MissingGlyphs.clear();
	}
}

void Instance::BakeFonts()
{
	PROFILE_SCOPED()
	//	Output("Baking fonts\n");

	m_should_bake_fonts = false;

	if (m_pi_fonts.size() == 0) {
		//		Output("No fonts to bake.\n");
		return;
	}

	ClearFonts();

	// first bake tooltip/default font
	BakeFont(m_pi_fonts.at(std::make_pair("pionillium", 14)));

	for (auto &iter : m_pi_fonts) {
		// don't bake tooltip/default font again
		if (!(iter.first.first == "pionillium" && iter.first.second == 14))
			BakeFont(iter.second);
		//		Output("Fonts registered: %i\n", io.Fonts->Fonts.Size);
	}

	ImGui::GetIO().Fonts->Build();

	for (auto &iter : m_pi_fonts) {
		for (auto &glyph_data : iter.second.custom_glyphs()) {
			glyph_data.face->finishSVGFaceData(glyph_data.font, iter.second.pixelsize(), glyph_data.svgData, glyph_data.glyphRects.get());
		}

		iter.second.custom_glyphs().clear();
	}

	for (ImVector<ImWchar> *scratchVec : m_glyphRanges) {
		delete scratchVec;
	}

	m_glyphRanges.clear();

	m_instanceRenderer->CreateFontsTexture();
}

void Instance::Uninit()
{
	PROFILE_SCOPED()
	for (auto tex : m_svg_textures) {
		delete tex;
	}

	switch (m_renderer->GetRendererType()) {
	default:
	case Graphics::RENDERER_DUMMY:
		return;
	case Graphics::RENDERER_OPENGL_3x:
		m_instanceRenderer->Shutdown();
		break;
	}

	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	delete[] m_ioIniFilename;
}

//
// PiGui::PiFace
//

ImFont *PiFace::addTTFFaceToAtlas(int pixelSize, ImFontConfig *config, ImVector<ImWchar> *ranges)
{
	float size = pixelSize * sizefactor();
	const std::string path = FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), ttfname());
	ImFont *f = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), size, config, ranges->Data);
	assert(f);

	return f;
}

ImFont *PiFace::addSVGFaceToAtlas(int pixelSize, ImFontConfig *config, ImVector<ImWchar> *ranges, RasterizeSVGResult *svgData, ImVector<int> *outGlyphRects)
{
	assert(config->MergeMode);

	ImFontAtlas *atlas = ImGui::GetIO().Fonts;
	ImFont *font = atlas->Fonts.back();

	// we'll stretch the icon/character size if we're rendering with a lower-resolution fallback
	// (e.g. while waiting for high-res version to be rendered)
	int glyphWidth = svgData->width / m_svgcolumns;
	int glyphHeight = svgData->height / m_svgrows;

	for (int idx = 0; idx < ranges->size() - 1; idx += 2) {
		ImWchar firstChar = std::max(ranges->Data[idx], m_loadrange.first);
		ImWchar lastChar = std::min(ranges->Data[idx + 1], m_loadrange.second);

		if (firstChar > m_loadrange.second || lastChar < m_loadrange.first)
			continue;

		for (ImWchar glyph = firstChar; glyph <= lastChar; glyph++) {
			int slotIdx = atlas->AddCustomRectFontGlyph(font, glyph, glyphWidth, glyphHeight, pixelSize);
			outGlyphRects->push_back(slotIdx);
		}
	}

	return font;
}

int RGBA32TexOffsetFromCoords(int x, int y, int pitch)
{
	return (pitch * y * 4) + (x * 4);
}

void PiFace::finishSVGFaceData(ImFont *font, int pixelSize, RasterizeSVGResult *svgData, ImVector<int> *glyphRects)
{
	ImFontAtlas *atlas = ImGui::GetIO().Fonts;

	// Ensure texture data pointer is available and in RGBA32
	uint8_t *texData;
	int texWidth;
	atlas->GetTexDataAsRGBA32(&texData, &texWidth, nullptr);

	int glyphWidth = svgData->width / m_svgcolumns;
	int glyphHeight = svgData->height / m_svgrows;

	for (int rectIdx : *glyphRects) {
		ImFontAtlasCustomRect *rect = atlas->GetCustomRectByIndex(rectIdx);
		int glyphIndex = rect->GlyphID - m_loadrange.first;

		int glyphX = (glyphIndex % m_svgcolumns) * glyphWidth;
		int glyphY = (glyphIndex / m_svgcolumns) * glyphHeight;

		for (int line = 0; line < rect->Height; line++) {
			memcpy(
				texData + RGBA32TexOffsetFromCoords(rect->X, rect->Y + line, texWidth),
				svgData->data.get() + RGBA32TexOffsetFromCoords(glyphX, glyphY + line, svgData->width),
				glyphWidth * 4 // RGBA32
			);
		}

		// Size the glyph according to the pixel size rather than the rendered size
		// (which might be different with fallback data)
		ImFontGlyph *glyph = &font->Glyphs[font->IndexLookup[rect->GlyphID]];
		glyph->X1 = pixelSize;
		glyph->Y1 = pixelSize;
	}
}

//
// PiGui::PiFont
//

bool PiFont::addGlyph(unsigned short glyph)
{
	PROFILE_SCOPED()
	for (auto &range : m_used_ranges) {
		if (range.first <= glyph && glyph <= range.second) {
			// if we already added it once and are trying to add it again,
			// it's invalid and not covered by any of the faces in this font
			// this avoids spurious rebakes if the font does not provide a glyph
			// in any of its faces
			return false;
		}
	}
	m_used_ranges.push_back(std::make_pair(glyph, glyph));
	return true;
}

void PiFont::describe(bool withFaces) const
{
	Log::Info("font {}:{}\n", name(), pixelsize());

	if (withFaces) {
		for (const PiFace &face : faces()) {
			if (face.isSvgFont())
				Log::Info("  - {} {}x{}\n", face.svgname(), face.svgCols(), face.svgRows());
			else
				Log::Info("  - {} {}\n", face.ttfname(), face.sizefactor());
		}
	}
}
