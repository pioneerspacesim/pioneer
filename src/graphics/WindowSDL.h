#ifndef GRAPHICS_WINDOW_SDL_H
#define GRAPHICS_WINDOW_SDL_H

#include <SDL_stdinc.h>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

namespace Graphics {

class WindowSDL {
public:
	WindowSDL(const std::string &name, Uint32 w, Uint32 h);
    ~WindowSDL();

	void SetGrab(bool grabbed);

private:
	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
};

}

#endif
