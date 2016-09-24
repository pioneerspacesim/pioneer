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

	static ImTextureID icons;

	PiGui() {
	}
	// PiGui(const PiGui &other) {
	// }
	LuaRef GetHandlers() const { return m_handlers; }
	LuaRef GetKeys() const { return m_keys; }
	
	void RenderHUD(double delta) {
		ScopedTable(m_handlers).Call<bool>("HUD", delta);
	}
	void Init(SDL_Window *window);
	void Uninit() {
		m_handlers.Unref();
		m_keys.Unref();
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

	static void *makeTexture(unsigned char *pixels, int width, int height)
	{
			GLint last_texture;
			GLuint result;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGenTextures(1, &result);
			glBindTexture(GL_TEXTURE_2D, result);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, last_texture);
			Output("texture id: %i\n", result);
			return reinterpret_cast<void*>(result);
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
	LuaRef m_keys;
};
