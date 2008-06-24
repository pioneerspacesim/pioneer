#ifndef _GUISCREEN_H
#define _GUISCREEN_H

#include "Gui.h"
#include <list>

class FontFace;

namespace Gui {
	class Screen {
	public:
		static void Init(int real_width, int real_height, int ui_width, int ui_height);
		static void Draw();
		static void AddBaseWidget(Widget *w);
		static void RemoveBaseWidget(Widget *w);
		static void OnClick(SDL_MouseButtonEvent *e);
		static void OnKeyDown(const SDL_keysym *sym);
		static void RenderString(const std::string &s);
		static void PutClickableLabel(const std::string &s, float x, float y, sigc::slot<void, const Gui::MouseButtonEvent*> slot);
		static void RenderLabel(const std::string &s, float x, float y);
		static void EnterOrtho();
		static void LeaveOrtho();
		static int GetWidth() { return width; }
		static int GetHeight() { return height; }
		// gluProject but fixes UI/screen size mismatch
		static GLint Project(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble *winY, GLdouble *winZ);
		friend void Widget::SetShortcut(SDLKey key, SDLMod mod);
	private:
		struct LabelPos {
			LabelPos(float _x, float _y): x(_x), y(_y) {}
			float x, y;
			sigc::signal<void, const Gui::MouseButtonEvent*> onClick;
		};
		static std::vector<LabelPos> labelPositions;
		static void OnClickTestLabels(const Gui::MouseButtonEvent &ev);
		static bool CanPutLabel(float x, float y);
		static void AddShortcutWidget(Widget *w);

		static bool initted;
		static int width, height;
		static int realWidth, realHeight;
		static float invRealWidth, invRealHeight;
		static std::list<Widget*> widgets;
		static std::list<Widget*> kbshortcut_widgets;
		static FontFace *font;
		static float font_xsize;
		static float font_ysize;
	};
}

#endif /* _GUISCREEN_H */
