#include "RefCounted.h"
#include "graphics/opengl/RendererGL.h"
#include "Lua.h"
#include "LuaRef.h"
#include "LuaTable.h"
#include "FileSystem.h"
#include "imgui.h"
#include "imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h"

class PiGui : public RefCounted {
public:
	static ImFont *pionillium12;
	static ImFont *pionillium15;
	static ImFont *pionillium18;
	static ImFont *pionillium30;
	static ImFont *pionillium36;
	static ImFont *pionicons12;
	//	static ImFont *pionicons18;
	static ImFont *pionicons30;
	
	PiGui() {
	}
	// PiGui(const PiGui &other) {
	// }
	LuaRef GetHandlers() const { return m_handlers; }
	void RenderHUD(double delta) {
		ScopedTable(m_handlers).Call<bool>("HUD", delta);
	}
	void Init(SDL_Window *window) {
		m_handlers.Unref();
		
		lua_State *l = Lua::manager->GetLuaState();
		lua_newtable(l);
		m_handlers = LuaRef(l, -1);
		
 		ImGui_ImplSdlGL3_Init(window);
		ImGuiIO &io = ImGui::GetIO();
		static unsigned short glyph_ranges[] = { 0x1, 0x3c0, 0x0, 0x0 };
		pionillium12 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/PionilliumText22L-Medium.ttf").c_str(), 12.0f, nullptr, glyph_ranges);
		pionillium15 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/PionilliumText22L-Medium.ttf").c_str(), 15.0f, nullptr, glyph_ranges);
		pionillium18 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/PionilliumText22L-Medium.ttf").c_str(), 18.0f, nullptr, glyph_ranges);
		pionillium30 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/PionilliumText22L-Medium.ttf").c_str(), 30.0f, nullptr, glyph_ranges);
		pionillium36 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/PionilliumText22L-Medium.ttf").c_str(), 36.0f, nullptr, glyph_ranges);
		pionicons12 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/Pionicons.ttf").c_str(), 12.0f, nullptr, glyph_ranges);
		//		pionicons18 = io.Fonts->AddFontFromFileTTF("data/fonts/Pionicons.ttf", 18.0f, nullptr, glyph_ranges);
		pionicons30 = io.Fonts->AddFontFromFileTTF((FileSystem::GetDataDir() + "fonts/Pionicons.ttf").c_str(), 30.0f, nullptr, glyph_ranges);
 	}
	void Uninit() {
		m_handlers.Unref();
	}
 	static void NewFrame(SDL_Window *window) {
 		ImGui_ImplSdlGL3_NewFrame(window);
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		ImGui::GetIO().MouseDrawCursor = true;
 	}
	static void Render() {
		ImGui::Render();
	}
	static bool ProcessEvent(SDL_Event *event) {
		return ImGui_ImplSdlGL3_ProcessEvent(event);
	}

	static bool WantCaptureMouse() {
		return ImGui::GetIO().WantCaptureMouse;
	}

	static bool WantCaptureKeyboard() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}
	static int RadialPopupSelectMenu(const ImVec2& center, std::string popup_id, std::vector<std::string> items, ImFont *itemfont, std::vector<std::string> tooltips);
	static bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	void Cleanup();
private:
	LuaRef m_handlers;
};
// namespace PiGui = ImGui;

// namespace ImGui {
// 	static void Init(SDL_Window *window) {
// 		ImGui_ImplSdlGL3_Init(window);
// 	}
// 	static void NewFrame(SDL_Window *window) {
// 		ImGui_ImplSdlGL3_NewFrame(window);
// 	}

// }

// enum PiGuiCol_ {
// 	PiGuiCol_Text = ImGuiCol_Text,
// 	PiGuiCol_TextDisabled =     ImGuiCol_TextDisabled,
// 	PiGuiCol_WindowBg =     ImGuiCol_WindowBg,              // Background of normal windows
// 	PiGuiCol_ChildWindowBg =     ImGuiCol_ChildWindowBg,         // Background of child windows
// 	PiGuiCol_PopupBg =     ImGuiCol_PopupBg,               // Background of popups, menus, tooltips windows
// 	PiGuiCol_Border =     ImGuiCol_Border,
// 	PiGuiCol_BorderShadow =     ImGuiCol_BorderShadow,
// 	PiGuiCol_FrameBg =     ImGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
// 	PiGuiCol_FrameBgHovered =     ImGuiCol_FrameBgHovered,
// 	PiGuiCol_FrameBgActive =     ImGuiCol_FrameBgActive,
// 	PiGuiCol_TitleBg =     ImGuiCol_TitleBg,
// 	PiGuiCol_TitleBgCollapsed =     ImGuiCol_TitleBgCollapsed,
// 	PiGuiCol_TitleBgActive =     ImGuiCol_TitleBgActive,
// 	PiGuiCol_MenuBarBg =     ImGuiCol_MenuBarBg,
// 	PiGuiCol_ScrollbarBg =     ImGuiCol_ScrollbarBg,
// 	PiGuiCol_ScrollbarGrab =     ImGuiCol_ScrollbarGrab,
// 	PiGuiCol_ScrollbarGrabHovered =     ImGuiCol_ScrollbarGrabHovered,
// 	PiGuiCol_ScrollbarGrabActive =     ImGuiCol_ScrollbarGrabActive,
// 	PiGuiCol_ComboBg =     ImGuiCol_ComboBg,
// 	PiGuiCol_CheckMark =     ImGuiCol_CheckMark,
// 	PiGuiCol_SliderGrab =     ImGuiCol_SliderGrab,
// 	PiGuiCol_SliderGrabActive =     ImGuiCol_SliderGrabActive,
// 	PiGuiCol_Button =     ImGuiCol_Button,
// 	PiGuiCol_ButtonHovered =     ImGuiCol_ButtonHovered,
// 	PiGuiCol_ButtonActive =     ImGuiCol_ButtonActive,
// 	PiGuiCol_Header =     ImGuiCol_Header,
// 	PiGuiCol_HeaderHovered =     ImGuiCol_HeaderHovered,
// 	PiGuiCol_HeaderActive =     ImGuiCol_HeaderActive,
// 	PiGuiCol_Column =     ImGuiCol_Column,
// 	PiGuiCol_ColumnHovered =     ImGuiCol_ColumnHovered,
// 	PiGuiCol_ColumnActive =     ImGuiCol_ColumnActive,
// 	PiGuiCol_ResizeGrip =     ImGuiCol_ResizeGrip,
// 	PiGuiCol_ResizeGripHovered =     ImGuiCol_ResizeGripHovered,
// 	PiGuiCol_ResizeGripActive =     ImGuiCol_ResizeGripActive,
// 	PiGuiCol_CloseButton =     ImGuiCol_CloseButton,
// 	PiGuiCol_CloseButtonHovered =     ImGuiCol_CloseButtonHovered,
// 	PiGuiCol_CloseButtonActive =     ImGuiCol_CloseButtonActive,
// 	PiGuiCol_PlotLines =     ImGuiCol_PlotLines,
// 	PiGuiCol_PlotLinesHovered =     ImGuiCol_PlotLinesHovered,
// 	PiGuiCol_PlotHistogram =     ImGuiCol_PlotHistogram,
// 	PiGuiCol_PlotHistogramHovered =     ImGuiCol_PlotHistogramHovered,
// 	PiGuiCol_TextSelectedBg =     ImGuiCol_TextSelectedBg,
// 	PiGuiCol_ModalWindowDarkening =     ImGuiCol_ModalWindowDarkening,  // darken entire screen when a modal window is active
// 	PiGuiCol_COUNT =     ImGuiCol_COUNT
// };

// typedef struct ImGuiStyle PiGuiStyle;

// enum PiGuiSetCond_
// {
// 	PiGuiSetCond_Always = ImGuiSetCond_Always, // Set the variable
// 	PiGuiSetCond_Once = ImGuiSetCond_Once, // Set the variable once per runtime session (only the first call with succeed)
// 	PiGuiSetCond_FirstUseEver = ImGuiSetCond_FirstUseEver, // Set the variable if the window has no saved data (if doesn't exist in the .ini file)
// 	PiGuiSetCond_Appearing = ImGuiSetCond_Appearing  // Set the variable if the window is appearing after being hidden/inactive (or the first time)
// };

// enum PiGuiSelectableFlags_
// {
//     // Default: 0
// 	PiGuiSelectableFlags_DontClosePopups = ImGuiSelectableFlags_DontClosePopups,   // Clicking this don't close parent popup window
// 	PiGuiSelectableFlags_SpanAllColumns = ImGuiSelectableFlags_SpanAllColumns,   // Selectable frame can span all columns (text will still fit in current column)
// 	PiGuiSelectableFlags_AllowDoubleClick = ImGuiSelectableFlags_AllowDoubleClick    // Generate press events on double clicks too
// };

// enum PiGuiWindowFlags_
// {
//     // Default: 0
// 	PiGuiWindowFlags_NoTitleBar = ImGuiWindowFlags_NoTitleBar,   // Disable title-bar
// 	PiGuiWindowFlags_NoResize = ImGuiWindowFlags_NoResize,   // Disable user resizing with the lower-right grip
// 	PiGuiWindowFlags_NoMove = ImGuiWindowFlags_NoMove,   // Disable user moving the window
// 	PiGuiWindowFlags_NoScrollbar = ImGuiWindowFlags_NoScrollbar,   // Disable scrollbars (window can still scroll with mouse or programatically)
// 	PiGuiWindowFlags_NoScrollWithMouse = ImGuiWindowFlags_NoScrollWithMouse,   // Disable user vertically scrolling with mouse wheel
// 	PiGuiWindowFlags_NoCollapse = ImGuiWindowFlags_NoCollapse,   // Disable user collapsing window by double-clicking on it
// 	PiGuiWindowFlags_AlwaysAutoResize = ImGuiWindowFlags_AlwaysAutoResize,   // Resize every window to its content every frame
// 	PiGuiWindowFlags_ShowBorders = ImGuiWindowFlags_ShowBorders,   // Show borders around windows and items
// 	PiGuiWindowFlags_NoSavedSettings = ImGuiWindowFlags_NoSavedSettings,   // Never load/save settings in .ini file
// 	PiGuiWindowFlags_NoInputs = ImGuiWindowFlags_NoInputs,   // Disable catching mouse or keyboard inputs
// 	PiGuiWindowFlags_MenuBar = ImGuiWindowFlags_MenuBar,  // Has a menu-bar
// 	PiGuiWindowFlags_HorizontalScrollbar = ImGuiWindowFlags_HorizontalScrollbar,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
// 	PiGuiWindowFlags_NoFocusOnAppearing = ImGuiWindowFlags_NoFocusOnAppearing,  // Disable taking focus when transitioning from hidden to visible state
// 	PiGuiWindowFlags_NoBringToFrontOnFocus = ImGuiWindowFlags_NoBringToFrontOnFocus,  // Disable bringing window to front when taking focus (e.g. clicking on it or programatically giving it focus)
// 	PiGuiWindowFlags_AlwaysVerticalScrollbar = ImGuiWindowFlags_AlwaysVerticalScrollbar,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
// 	PiGuiWindowFlags_AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
// 	PiGuiWindowFlags_AlwaysUseWindowPadding = ImGuiWindowFlags_AlwaysUseWindowPadding,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
//     // [Internal]
// 	PiGuiWindowFlags_ChildWindow = ImGuiWindowFlags_ChildWindow,  // Don't use! For internal use by BeginChild()
// 	PiGuiWindowFlags_ChildWindowAutoFitX = ImGuiWindowFlags_ChildWindowAutoFitX,  // Don't use! For internal use by BeginChild()
// 	PiGuiWindowFlags_ChildWindowAutoFitY = ImGuiWindowFlags_ChildWindowAutoFitY,  // Don't use! For internal use by BeginChild()
// 	PiGuiWindowFlags_ComboBox = ImGuiWindowFlags_ComboBox,  // Don't use! For internal use by ComboBox()
// 	PiGuiWindowFlags_Tooltip = ImGuiWindowFlags_Tooltip,  // Don't use! For internal use by BeginTooltip()
// 	PiGuiWindowFlags_Popup = ImGuiWindowFlags_Popup,  // Don't use! For internal use by BeginPopup()
// 	PiGuiWindowFlags_Modal = ImGuiWindowFlags_Modal,  // Don't use! For internal use by BeginPopupModal()
// 	PiGuiWindowFlags_ChildMenu = ImGuiWindowFlags_ChildMenu   // Don't use! For internal use by BeginMenu()
// };
