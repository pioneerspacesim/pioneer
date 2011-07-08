#ifndef _GUISCREEN_H
#define _GUISCREEN_H

#include "Gui.h"
#include "FontManager.h"
#include "TextureFont.h"
#include <list>

namespace Gui {
	class Screen {
	public:
		static void Init(int real_width, int real_height, int ui_width, int ui_height);
		static void Draw();
		static void ShowBadError(const char *msg);
		static void AddBaseWidget(Widget *w, int x, int y);
		static void RemoveBaseWidget(Widget *w);
		static void OnMouseMotion(SDL_MouseMotionEvent *e);
		static void OnClick(SDL_MouseButtonEvent *e);
		static void OnKeyDown(const SDL_keysym *sym);
		static void OnKeyUp(const SDL_keysym *sym);
		static void RenderString(const std::string &s, float xoff, float yoff);
		static void MeasureString(const std::string &s, float &w, float &h);
		static void RenderMarkup(const std::string &s);
		static void RenderLabel(const std::string &s, float x, float y);
		static void EnterOrtho();
		static void LeaveOrtho();
		static int GetWidth() { return width; }
		static int GetHeight() { return height; }
		static float GetFontHeight();
		// gluProject but fixes UI/screen size mismatch
		static bool Project(const vector3d &in, vector3d &out);
		friend void Widget::SetShortcut(SDLKey key, SDLMod mod);
		friend Widget::~Widget();
		static bool IsBaseWidget(const Widget *);
		static void GetCoords2Pixels(float scale[2]) {
			scale[0] = fontScale[0];
			scale[1] = fontScale[1];
		}
		static const float* GetCoords2Pixels() { return fontScale; }
		static TextureFont *GetFont() { return font; }
		static void SetFocused(Widget *w);
		static bool IsFocused(Widget *w) {
			return w == focusedWidget;
		}

		static FontManager *GetFontManager() { return &s_fontManager; }

	private:
		static void AddShortcutWidget(Widget *w);
		static void RemoveShortcutWidget(Widget *w);
		static void SDLEventCoordToScreenCoord(int sdlev_x, int sdlev_y, float *x, float *y);

		static bool initted;
		static int width, height;
		static int realWidth, realHeight;
		static float invRealWidth, invRealHeight;
		static std::list<Widget*> kbshortcut_widgets;
		static std::list<Widget*> mouseHoveredWidgets;
		static TextureFont *font;
		static float fontScale[2];
		static Gui::Fixed *baseContainer;
		static Gui::Widget *focusedWidget;
		static void OnDeleteFocusedWidget();
		static GLdouble modelMatrix[16];
		static GLdouble projMatrix[16];
		static GLint viewport[4];

		static FontManager s_fontManager;
	};
}

#endif /* _GUISCREEN_H */
