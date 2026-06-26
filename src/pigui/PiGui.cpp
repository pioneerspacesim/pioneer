// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGui.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "Pi.h"
#include "PiGuiRenderer.h"

#include "core/Log.h"
#include "core/TaskGraph.h"
#include "core/StringUtils.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"

#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "profiler/Profiler.h"
#include "utils.h"

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

struct PiGui::SVGFontFile {
	ImWchar start_codepoint; // unicode codepoint to load the first icon at
	uint16_t grid_w; // number of columns in the grid
	uint16_t grid_h; // number of rows in the grid
	PiGui::Instance *inst;
	std::string filename;
	std::string path;
};

struct PiGui::SVGFontBaked {
	SVGFontFile *file;
	uint32_t pixel_sz; // output pixel size of an individual icon (always square)
	uint32_t px_w; // pixel width to rasterize the file at
	uint32_t px_h; // pixel height to rasterize the file at
	uint32_t icon_w; // pitch between icon row starts, in pixels
	uint32_t icon_h; // pitch between icon row starts, in pixels
	uint32_t raster_idx; // index of the SVGFontRasterized in the appropriate array
	uint8_t *data = nullptr; // pointer to the rasterized data for this font
	std::vector<ImWchar> pendingGlyphs; // glyphs that need to be uploaded to the font atlas
};

struct RasterizeSVGResult : SVGFontBaked {};

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

	void OnExecute(TaskRange range) override
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
	RasterizeSVGTask(const std::string &filename, int width, int height, Graphics::Texture *outputTexture) :
		filename(filename),
		path(filename),
		width(width),
		height(height),
		texture(outputTexture)
	{
	}

	// Rasterize an SVG file to CPU buffer for use with font data
	RasterizeSVGTask(const std::string &filename, int width, int height) :
		filename(filename),
		width(width),
		height(height),
		texture(nullptr)
	{
		path = FileSystem::JoinPathBelow(FileSystem::GetDataDir(), filename);
		SetOwner(this);
	}

	bool LoadFile()
	{
		PROFILE_SCOPED();

		image = nsvgParseFromFile(path.c_str(), "px", 96.0f);
		if (image == NULL) {
			Log::Error("Could not open SVG image {}.\n", path);
			return false;
		}

		return true;
	}

	void OnExecute(TaskRange range) override
	{
		PROFILE_SCOPED()

		if (!LoadFile())
			return;

		size_t stride = width * 4;
		imageData = new uint8_t[stride * height];

		if (!imageData) {
			Log::Error("Couldn't allocate memory for SVG image {}.\n", path);
			return;
		}

		memset(imageData, 0, stride * height);

		NSVGrasterizer *rast = nsvgCreateRasterizer();
		if (!rast) {
			Log::Error("Couldn't create SVG rasterizer for SVG image {}.\n", path);
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

	uint8_t *GetImageData() { return imageData; }

public:
	std::string filename;
	int width;
	int height;

private:
	std::string path;
	Graphics::Texture *texture;
	uint8_t *imageData;
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
	style.Colors[ImGuiCol_TabSelected] = ImColor(60, 133, 224);
	style.Colors[ImGuiCol_TabHovered] = ImColor(66, 150, 250);
}

ImFontBaked *GetFontBakedFromID(ImGuiID id, ImFontAtlas *atlas)
{
	ImFontAtlasBuilder *builder = atlas->Builder;
	return *reinterpret_cast<ImFontBaked **>(builder->BakedMap.GetVoidPtrRef(id));
}

struct PiGui::PiSVGLoader {

	static bool FontSrcContainsGlyph(ImFontAtlas *atlas, ImFontConfig *src, ImWchar codepoint)
	{
		auto *ff = reinterpret_cast<SVGFontFile *>(src->FontData);
		return codepoint >= ff->start_codepoint && codepoint < ff->grid_w * ff->grid_h + ff->start_codepoint;
	}

	static bool FontBakedInit(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src)
	{
		auto *ff = reinterpret_cast<SVGFontFile *>(src->FontData);

		// currently don't support horizontal oversampling but it's an option for the future...
		int oversample_w = 1;
		int oversample_h = 1;

		SVGFontBaked *raster = new (loader_data_for_baked_src) SVGFontBaked();
		raster->file = ff;
		raster->pixel_sz = baked->Size * src->RasterizerDensity * baked->RasterizerDensity;
		raster->icon_w = raster->pixel_sz * oversample_w;
		raster->icon_h = raster->pixel_sz * oversample_h;
		raster->px_w = raster->icon_w * ff->grid_w;
		raster->px_h = raster->icon_h * ff->grid_h;

		ff->inst->RequestSVGFaceData({ src, baked->BakedId }, raster);
		return true;
	}

    static void FontBakedDestroy(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src)
	{
		auto *ff = reinterpret_cast<SVGFontFile *>(src->FontData);
		ff->inst->CancelSVGFaceData({ src, baked->BakedId });
	}

	static void FontUploadGlyph(ImFontAtlas *atlas, ImFontConfig *src, ImFontBaked *baked, ImFontGlyph *glyph, SVGFontBaked *svg)
	{
		ImTextureRect *r = ImFontAtlasPackGetRect(atlas, glyph->PackId);

		const uint32_t index = glyph->Codepoint - svg->file->start_codepoint;

		const uint32_t src_x = index % svg->file->grid_w;
		const uint32_t src_y = index / svg->file->grid_w;

		const uint32_t pitch_x = svg->icon_w * 4;
		const uint32_t pitch_y = svg->icon_h * svg->px_w * 4;

		uint8_t *src_pixels = svg->data + src_y * pitch_y + src_x * pitch_x;
		ImFontAtlasBakedSetFontGlyphBitmap(atlas, baked, src, glyph, r, src_pixels, ImTextureFormat_RGBA32, svg->px_w * 4);
	}

    static bool FontBakedLoadGlyph(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src, ImWchar codepoint, ImFontGlyph* out_glyph, float* out_advance_x)
	{
		auto *ff = reinterpret_cast<SVGFontFile *>(src->FontData);

		int index = codepoint - ff->start_codepoint;
		if (index < 0 || index >= ff->grid_w * ff->grid_h)
			return false;

		if (out_advance_x) {
			*out_advance_x = baked->Size;
			return true;
		}

		auto *raster = reinterpret_cast<SVGFontBaked *>(loader_data_for_baked_src);

		out_glyph->Codepoint = codepoint;
		out_glyph->AdvanceX = baked->Size;
		const int w = raster->icon_w;
		const int h = raster->icon_h;

		ImFontAtlasRectId pack_id = ImFontAtlasPackAddRect(atlas, w, h);
		if (pack_id == ImFontAtlasRectId_Invalid) {
			return false;
		}

		ImTextureRect *r = ImFontAtlasPackGetRect(atlas, pack_id);

		out_glyph->X0 = 0;
		out_glyph->Y0 = 0;
		out_glyph->X1 = baked->Size;
		out_glyph->Y1 = baked->Size;
		out_glyph->Visible = true;
		out_glyph->PackId = pack_id;

		if (raster->data) {
			FontUploadGlyph(atlas, src, baked, out_glyph, raster);
		} else {
			// defer uploading the glyph pixels until they've been rasterized at the size we need
			raster->pendingGlyphs.push_back(codepoint);
		}

		return true;
	}

	static const ImFontLoader *GetFontLoader()
	{
		static ImFontLoader svgLoader {};
		svgLoader.Name = "PiSVGLoader";
		svgLoader.FontSrcContainsGlyph = &FontSrcContainsGlyph;
		svgLoader.FontBakedInit = &FontBakedInit;
		svgLoader.FontBakedDestroy = &FontBakedDestroy;
		svgLoader.FontBakedLoadGlyph = &FontBakedLoadGlyph;
		svgLoader.FontBakedSrcLoaderDataSize = sizeof(SVGFontBaked);
		return &svgLoader;
	}
};

//
//	PiGui::Instance
//

Instance::Instance(GuiApplication *app) :
	m_app(app),
	m_debugStyle(),
	m_debugStyleActive(false)
{
}

void Instance::LoadFontDefinitionFromFile(const std::string &filePath)
{
	PROFILE_SCOPED()

	Json fontInfo = JsonUtils::LoadJsonDataFile(filePath);

	std::string fontName = fontInfo["name"].get<std::string>();
	if (fontName.empty()) {
		Log::Warning("Malformed font definition {}, not loading.", filePath);
		return;
	}

	ImGuiIO &io = ImGui::GetIO();
	ImFont *font = nullptr;

	ImFontConfig cfg = {};
	cfg.SizePixels = fontInfo.value("sizePixels", 12.f);

	for (auto &entry : fontInfo["faces"]) {
		if (!entry.is_object())
			continue;

		if (entry["fontFile"].is_string()) {
			std::string filename = entry["fontFile"].get<std::string>();
			float size = cfg.SizePixels * entry.value<float>("scale", 1.0);

			FileSystem::FileInfo info = FileSystem::gameDataFiles.Lookup(FileSystem::JoinPathBelow("fonts", filename));
			RefCountedPtr<FileSystem::FileData> fd = info.Read();

			// will be owned by the font atlas
			char *font_data = new char[fd->GetSize()];
			std::memcpy(font_data, fd->GetData(), fd->GetSize());

			snprintf(cfg.Name, sizeof(cfg.Name) - 1, "%s", filename.c_str());

			cfg.FontLoader = nullptr;
			cfg.FontData = nullptr;
			cfg.FontDataOwnedByAtlas = true;

			font = io.Fonts->AddFontFromMemoryTTF(font_data, fd->GetSize(), size, &cfg);

			cfg.MergeMode = true;
		} else if (entry["iconFile"].is_string()) {

			std::string iconFile = entry["iconFile"].get<std::string>();
			Json iconData = JsonUtils::LoadJsonDataFile(iconFile);

			if (!m_svgSources.count(iconFile)) {
				SVGFontFile ff {};
				ff.inst = this;
				ff.filename = iconData["svgFile"].get<std::string>();
				ff.grid_w = iconData["grid"][0].get<uint32_t>();
				ff.grid_h = iconData["grid"][1].get<uint32_t>();

				uint32_t rangeBase = 0xF000;
				sscanf(iconData.value("rangeBase", "0xF000").c_str(), "%x", &rangeBase);
				ff.start_codepoint = rangeBase;

				m_svgSources.try_emplace(iconFile, std::move(ff));
			}

			snprintf(cfg.Name, sizeof(cfg.Name) - 1, "%s", m_svgSources[iconFile].filename.c_str());

			cfg.FontLoader = PiSVGLoader::GetFontLoader();
			cfg.FontData = &m_svgSources[iconFile];
			cfg.FontDataOwnedByAtlas = false;

			font = io.Fonts->AddFont(&cfg);

			cfg.MergeMode = true;
		}
	}

	if (!font) {
		Log::Warning("Font definition {} has no valid faces.", filePath);
		return;
	}

	if (font) {
		m_fontMap.emplace(fontName, font);
	}
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

ImFont *Instance::GetFont(const std::string &name)
{
	PROFILE_SCOPED()
	assert(!m_fontMap.empty());

	auto iter = m_fontMap.find(name);
	if (iter == m_fontMap.end()) {
		Log::Error("GetFont: cannot find font {}, substituting font {}.", name, m_fontMap.begin()->first);
		m_fontMap[name] = m_fontMap.begin()->second;
	}

	return m_fontMap.at(name);
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
	for (auto &entry : m_fontMap) {
		Log::Info("{} (default size: {})", entry.first, entry.second->LegacySize);
	}

	if (!m_fontMap.count("pionillium")) {
		Log::Fatal("Missing font definition for required font 'pionillium'. Pioneer cannot proceed.");
	}

	ImGuiIO &io = ImGui::GetIO();
	// Apply the base style
	ImGui::StyleColorsDark();

	StyleColorsDarkPlus(m_debugStyle);

	// Disable ctrl+tab / ctrl+shift+tab window switching
	// https://github.com/ocornut/imgui/issues/3255#issuecomment-2529061532
	ImGui::GetCurrentContext()->ConfigNavWindowingKeyNext = 0; // replace the default which is `ImGuiMod_Ctrl | ImGuiKey_Tab`
	ImGui::GetCurrentContext()->ConfigNavWindowingKeyPrev = 0; // replace the default which is `ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab`.

	std::string imguiIni = FileSystem::JoinPath(FileSystem::GetUserDir(), "imgui.ini");

	m_ioIniFilename = new char[imguiIni.size() + 1];
	std::strncpy(m_ioIniFilename, imguiIni.c_str(), imguiIni.size() + 1);
	io.IniFilename = m_ioIniFilename;

	// Enable error recovery support
	io.ConfigErrorRecovery = true;
	io.ConfigErrorRecoveryEnableTooltip = true;
	io.ConfigErrorRecoveryEnableAssert = false;

	io.FontDefault = GetFont("pionillium");
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
	ImGuiIO &io = ImGui::GetIO();

	// Process all completed rasterization tasks and store their image data in the pending request
	for (auto &task : m_svgFontTasks) {

		if (task->IsComplete()) {

			for (SVGFontRasterized &raster : m_svgRasterData[task->filename]) {
				if (raster.width == task->width && raster.height == task->height) {
					raster.data.reset(task->GetImageData());
					break;
				}
			}

			delete task;
			task = nullptr;
		}
	}

	// Sort all erased tasks to the end of the list and erase them
	m_svgFontTasks.erase(std::remove(m_svgFontTasks.begin(), m_svgFontTasks.end(), nullptr), m_svgFontTasks.end());

	// Check all pending uploads to see if they have data yet...
	for (auto &[font_pair, font_data] : m_pendingUploads) {
		SVGFontRasterized &raster = m_svgRasterData[font_data->file->filename][font_data->raster_idx];

		if (raster.data) {
			assert(font_data->data == nullptr);
			font_data->data = raster.data.get();

			ImFontBaked *baked = GetFontBakedFromID(font_pair.second, io.Fonts);

			for (ImWchar codepoint : font_data->pendingGlyphs) {
				ImFontGlyph *glyph = baked->FindGlyph(codepoint);
				ImTextureRect *r = ImFontAtlasPackGetRect(io.Fonts, glyph->PackId);

				PiSVGLoader::FontUploadGlyph(io.Fonts, font_pair.first, baked, glyph, font_data);
			}

			font_data->pendingGlyphs.clear();
			font_data = nullptr;
		}
	}

	// Erase all nullptr values in m_pendingUploads (data is present and glyphs will be uploaded directly)
	erase_mapped_val(m_pendingUploads, nullptr);

	switch (m_renderer->GetRendererType()) {
	default:
	case Graphics::RENDERER_DUMMY:
		Error("RENDERER_DUMMY is not a valid renderer, aborting.");
		return;
	case Graphics::RENDERER_OPENGL_3x:
		m_instanceRenderer->NewFrame();
		break;
	}
	ImGui_ImplSDL2_NewFrame();
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

void Instance::RequestSVGFaceData(FontPair pair, SVGFontBaked *font_data)
{
	const std::string &filename = font_data->file->filename;

	for (auto &result : m_svgRasterData[filename]) {
		if (result.width == font_data->px_w && result.height == font_data->px_h) {
			font_data->data = result.data.get();
			// If the data is already available, no pending uploads will accumulate.
			// Don't register this font for a pending upload or set its raster_idx.
			return;
		}
	}

	SVGFontRasterized raster = {};
	raster.width = font_data->px_w;
	raster.height = font_data->px_h;
	raster.data = nullptr;

	font_data->raster_idx = m_svgRasterData[filename].size();
	m_svgRasterData[filename].emplace_back(std::move(raster));

	Log::Info("Queuing rasterization of font {}@{}", filename, font_data->px_w);
	RasterizeSVGTask *rasterTask = new RasterizeSVGTask(filename, font_data->px_w, font_data->px_h);
	// This font may accumulate pending glyph uploads while the SVG data is being rasterized.
	m_pendingUploads[pair] = font_data;

	m_app->GetTaskGraph()->QueueTask(rasterTask);
	m_svgFontTasks.push_back(rasterTask);
}

void Instance::CancelSVGFaceData(FontPair for_font)
{
	// The underlying baked font is being discarded, so we erase any pending glyph uploads
	m_pendingUploads.erase(for_font);
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
