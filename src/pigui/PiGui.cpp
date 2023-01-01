// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGui.h"
#include "FileSystem.h"
#include "Input.h"
#include "Pi.h"
#include "PiGuiRenderer.h"

#include "core/TaskGraph.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "imgui/imgui.h"

#include "imgui/examples/imgui_impl_sdl.h"

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
class UpdateImageTask : public Task
{
public:
	Graphics::Texture *texture;
	const unsigned char *imageData;

	UpdateImageTask(Graphics::Texture *tex, const unsigned char *data) :
		texture(tex),
		imageData(data)
	{}

	virtual void OnExecute(TaskRange range) override
	{
		PROFILE_SCOPED()

		const Graphics::TextureDescriptor &desc = texture->GetDescriptor();
		texture->Update(imageData, desc.dataSize, desc.format);
		delete[] imageData;
	}
};

// Run SVG loading and rasterization on a separate thread, defer GPU upload until end-of-frame.
class RasterizeSVGTask : public Task
{
public:
	RasterizeSVGTask(std::string filename, int width, int height, Graphics::Texture *outputTexture) :
		filename(filename),
		width(width),
		height(height),
		texture(outputTexture)
	{}

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
		uint8_t *imageData = new uint8_t[stride*height];

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

		Pi::GetApp()->GetTaskGraph()->QueueTaskPinned(new UpdateImageTask(texture, imageData));
	}

private:
	std::string filename;
	int width;
	int height;
	Graphics::Texture *texture;
	NSVGimage *image;
};

static Graphics::Texture* makeSVGTexture(Graphics::Renderer *renderer, int width, int height)
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

Instance::Instance() :
	m_should_bake_fonts(true),
	m_debugStyle(),
	m_debugStyleActive(false)
{
	// TODO: clang-format doesn't like list initializers inside function calls
	// clang-format off

	// The main face of the font should go first in the list, because:
	//
	// - during the first initialization, only the first face is used to bake the
	// font ( see Instance::AddFont )
	//
	// - when a non-standard glyph is found in the text, a search is started in
	// the faces, and the faces are scanned in the order of this list
	// ( see Instance::AddGlyph )
	//
	// - the default imgui glyph range ( 0x20 .. 0xFF ) is always baked from the
	// first face, that has some ranges defined
	// ( see Instance::BakeFont )

	PiFont uiheading("orbiteer", {
		PiFace("Orbiteer-Bold.ttf", 1.0), // imgui only supports 0xffff, not 0x10ffff
		PiFace("DejaVuSans.ttf", /*18.0/20.0*/ 1.2),
		PiFace("wqy-microhei.ttc", 1.0)
	});
	AddFontDefinition(uiheading);

	PiFont guifont("pionillium", {
		PiFace("PionilliumText22L-Medium.ttf", 1.0),
		PiFace("DejaVuSans.ttf", 13.0 / 14.0),
		PiFace("wqy-microhei.ttc", 1.0)
	});
	AddFontDefinition(guifont);
	// clang-format on

	// Output("Fonts:\n");
	for (auto entry : m_font_definitions) {
		//		Output("  entry %s:\n", entry.first.c_str());
		entry.second.describe();
	}

	// ensure the tooltip font exists
	GetFont("pionillium", 14);

	StyleColorsDarkPlus(m_debugStyle);
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
	// as a result, we look at all the faces of the font in turn, for the
	// presence of this glyph.
	PiFont &pifont = pifont_iter->second;
	for (PiFace &face : pifont.faces()) {
		if (face.isValidGlyph(glyph)) {
			// and as soon as we find it, we define the glyph..glyph range in this face
			face.addGlyph(glyph);
			// and enable the flag that all fonts should be rebaked
			// ( see Instance::BakeFont )
			m_should_bake_fonts = true;
			return;
		}
	}
	Error("No face in font %s handles glyph %i\n", pifont.name().c_str(), glyph);
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

	PiFont &pifont = iter->second;
	pifont.setPixelsize(size);
	// here we add the range 0x0020 .. 0x0020 to the first face of the font
	// the other faces of this font do not yet have ranges, so they will not be
	// used for baking for the first time
	pifont.faces()[0].addGlyph(0x20);
	m_pi_fonts[std::make_pair(name, size)] = pifont;

	m_should_bake_fonts = true;

	return m_fonts[std::make_pair(name, size)];
}

void Instance::RefreshFontsTexture()
{
	PROFILE_SCOPED()
	ImGui::GetIO().Fonts->Build();
	m_instanceRenderer->CreateFontsTexture();
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

// this function rasterizes a specific font
void Instance::BakeFont(PiFont &font)
{
	PROFILE_SCOPED()
	ImGuiIO &io = ImGui::GetIO();
	ImFont *imfont = nullptr;
	for (PiFace &face : font.faces()) {
		ImFontConfig config;
		config.MergeMode = true;
		float size = font.pixelsize() * face.sizefactor();
		const std::string path = FileSystem::JoinPath(FileSystem::JoinPath(FileSystem::GetDataDir(), "fonts"), face.ttfname());
		//		Output("- baking face %s at size %f\n", path.c_str(), size);
		face.sortUsedRanges();
		// note that if there are no ranges at all in the face, it is ignored
		if (face.used_ranges().size() > 0) {
			face.m_imgui_ranges.clear();
			ImFontGlyphRangesBuilder gb;
			// but if at least one range is defined for a face, then we are trying to
			// add a default range to it
			gb.AddRanges(io.Fonts->GetGlyphRangesDefault());
			// ( default imgui glyph range - 0x0020 .. 0x00FF : Basic Latin + Latin Supplement )
			// it turns out that if ranges are defined in several faces of the font,
			// we try to add a default range to each. but since the repeated range
			// is ignored, the default range is baked only from the first face that
			// falls into this scope
			ImWchar gr[3] = { 0, 0, 0 };
			for (auto &range : face.used_ranges()) {
				// Output("Used range: %x - %x", range.first, range.second);
				gr[0] = range.first;
				gr[1] = range.second;
				gb.AddRanges(gr);
			}
			gb.BuildRanges(&face.m_imgui_ranges);
			ImFont *f = io.Fonts->AddFontFromFileTTF(path.c_str(), size, imfont == nullptr ? nullptr : &config, face.m_imgui_ranges.Data);
			assert(f);
			if (imfont != nullptr)
				assert(f == imfont);
			imfont = f;
		}
	}
	m_im_fonts[imfont] = std::make_pair(font.name(), font.pixelsize());
	// 	Output("setting %s %i to %p\n", font.name(), font.pixelsize(), imfont);
	m_fonts[std::make_pair(font.name(), font.pixelsize())] = imfont;
	if (!imfont->MissingGlyphs.empty()) {
		Output("WARNING: glyphs missing in shiny new font\n");
	}
	imfont->MissingGlyphs.clear();
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
	BakeFont(m_pi_fonts[std::make_pair("pionillium", 14)]);

	for (auto &iter : m_pi_fonts) {
		// don't bake tooltip/default font again
		if (!(iter.first.first == "pionillium" && iter.first.second == 14))
			BakeFont(iter.second);
		//		Output("Fonts registered: %i\n", io.Fonts->Fonts.Size);
	}

	RefreshFontsTexture();
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

bool PiFace::isValidGlyph(unsigned short glyph) const
{
	PROFILE_SCOPED()
	return (m_invalid_glyphs.count(glyph) == 0);
}

void PiFace::addGlyph(unsigned short glyph)
{
	PROFILE_SCOPED()
	// Output("- PiFace %s adding glyph 0x%x\n", ttfname().c_str(), glyph);
	for (auto &range : m_used_ranges) {
		if (range.first <= glyph && glyph <= range.second) {
			// Output(" - already added, not adding again\n");
			m_invalid_glyphs.insert(glyph); //if already added it once and trying to add it again, it's invalid
			return;
		}
	}
	//	Output(" - added\n");
	m_used_ranges.push_back(std::make_pair(glyph, glyph));
}

void PiFace::sortUsedRanges() const
{
	PROFILE_SCOPED()
	// sort by ascending lower end of range
	std::sort(m_used_ranges.begin(), m_used_ranges.end(), [](const std::pair<unsigned short, unsigned short> &a, const std::pair<unsigned short, unsigned short> &b) { return a.first < b.first; });
	// merge adjacent ranges
	std::vector<std::pair<unsigned short, unsigned short>> merged;
	std::pair<unsigned short, unsigned short> current(0xffff, 0xffff);
	for (auto &range : m_used_ranges) {
		//		Output("> checking 0x%x-0x%x\n", range.first, range.second);
		if (current.first == 0xffff && current.second == 0xffff)
			current = range;
		else {
			// if only a few are missing in range, just merge nontheless. +5 is 4 missing
			if (current.second + 5 >= range.first) { // (current.second + 1 == range.first)
				//				Output("> merging 0x%x-0x%x and 0x%x-0x%x\n", current.first, current.second, range.first, range.second);
				current.second = range.second;
			} else {
				//				Output("> pushing 0x%x-0x%x\n", current.first, current.second);
				merged.push_back(current);
				current = range;
			}
		}
	}
	if (current.first != 0xffff && current.second != 0xffff)
		merged.push_back(current);
	m_used_ranges.assign(merged.begin(), merged.end());
}
