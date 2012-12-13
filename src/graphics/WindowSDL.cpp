#include "WindowSDL.h"

#include "SDL.h"

namespace Graphics {

WindowSDL::WindowSDL(const std::string &name, Uint32 w, Uint32 h)
{
	m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	m_renderer = SDL_CreateRenderer(m_window, -1, 0);
}

WindowSDL::~WindowSDL()
{
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
}

void WindowSDL::SetGrab(bool grabbed)
{
	SDL_SetWindowGrab(m_window, SDL_bool(grabbed));
}

}
