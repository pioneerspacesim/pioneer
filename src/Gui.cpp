#include "libs.h"
#include "Gui.h"

namespace Gui {

namespace RawEvents {
	sigc::signal<void, SDL_MouseMotionEvent *> onMouseMotion;
	sigc::signal<void, SDL_MouseButtonEvent *> onMouseDown;
	sigc::signal<void, SDL_MouseButtonEvent *> onMouseUp;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
}

namespace Color {
	const float bg[] = { .25,.37,.63 };
	const float bgShadow[] = { .08,.12,.21 };
}

void HandleSDLEvent(SDL_Event *event)
{
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			Screen::OnClick(&event->button);
			RawEvents::onMouseDown.emit(&event->button);
			break;
		case SDL_MOUSEBUTTONUP:
			Screen::OnClick(&event->button);
			RawEvents::onMouseUp.emit(&event->button);
			break;
		case SDL_KEYDOWN:
			Screen::OnKeyDown(&event->key.keysym);
			RawEvents::onKeyDown.emit(&event->key);
			break;
		case SDL_KEYUP:
			RawEvents::onKeyUp.emit(&event->key);
			break;
		case SDL_MOUSEMOTION:
			Screen::OnMouseMotion(&event->motion);
			RawEvents::onMouseMotion.emit(&event->motion);
			break;
	}
}

struct TimerSignal {
	Uint32 goTime;
	sigc::signal<void> *sig;
};

static std::list<TimerSignal> g_timeSignals;

void AddTimer(Uint32 ms, sigc::signal<void> *s)
{
	TimerSignal _s;
	_s.goTime = SDL_GetTicks() + ms;
	_s.sig = s;
	g_timeSignals.push_back(_s);
}

void RemoveTimer(sigc::signal<void> *s)
{
	for (std::list<TimerSignal>::iterator i = g_timeSignals.begin();
		i != g_timeSignals.end();) {
		if ((*i).sig == s)
			i = g_timeSignals.erase(i);
		else ++i;
	}
}

static void ExpireTimers(Uint32 t)
{
	for (std::list<TimerSignal>::iterator i = g_timeSignals.begin();
		i != g_timeSignals.end();) {
		if (t >= (*i).goTime)
			i = g_timeSignals.erase(i);
		else ++i;
	}
}

void Draw()
{
	Uint32 t = SDL_GetTicks();
	// also abused like an update() function...
	for (std::list<TimerSignal>::iterator i = g_timeSignals.begin(); i != g_timeSignals.end(); ++i) {
		if (t >= (*i).goTime) (*i).sig->emit();
	}
	ExpireTimers(t);

	Screen::Draw();
}

void Init(int screen_width, int screen_height, int ui_width, int ui_height)
{
	Screen::Init(screen_width, screen_height, ui_width, ui_height);
}

}
