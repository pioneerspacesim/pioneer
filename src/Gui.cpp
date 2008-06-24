#include "libs.h"
#include "Gui.h"

namespace Gui {

namespace RawEvents {
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
	}
}

void Draw()
{
	Screen::Draw();
}

void Init(int screen_width, int screen_height, int ui_width, int ui_height)
{
	Screen::Init(screen_width, screen_height, ui_width, ui_height);
}

}
