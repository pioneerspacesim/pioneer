#ifndef GRAPHICS_WINDOW_SDL_H
#define GRAPHICS_WINDOW_SDL_H

#include "Graphics.h"

namespace Graphics {

class WindowSDL {
public:
	WindowSDL(const Graphics::Settings &settings, const std::string &name);
	~WindowSDL();

	int GetWidth() const;
	int GetHeight() const;

	void SetGrab(bool grabbed);

	void SwapBuffers();

private:
	SDL_Window *m_window;
	SDL_GLContext m_glContext;
};

}

#endif
