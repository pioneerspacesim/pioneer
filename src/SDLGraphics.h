#ifndef SDLGRAPHICS_H
#define SDLGRAPHICS_H

#include <SDL_stdinc.h>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

class SDLGraphics {
public:
	SDLGraphics(const std::string &name, Uint32 w, Uint32 h);
    ~SDLGraphics();

private:
	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
};

#endif
