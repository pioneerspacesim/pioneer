#ifndef GRAPHICS_WINDOW_SDL_H
#define GRAPHICS_WINDOW_SDL_H

#include "Graphics.h"

struct SDL_Window;
struct SDL_Renderer;

namespace Graphics {

class WindowSDL {
public:
	WindowSDL(const Graphics::Settings &settings, const std::string &name);
    ~WindowSDL();

	void SetGrab(bool grabbed);

	void SwapBuffers();

private:
	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
};

}

#endif
