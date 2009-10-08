#include "libs.h"
#include "Gui.h"

namespace Gui {

namespace RawEvents {
	sigc::signal<void, MouseMotionEvent *> onMouseMotion;
	sigc::signal<void, MouseButtonEvent *> onMouseDown;
	sigc::signal<void, MouseButtonEvent *> onMouseUp;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
}

namespace Color {
	const float bg[] = { .25f,.37f,.63f };
	const float bgShadow[] = { .08f,.12f,.21f };
	const float tableHeading[] = { .7f,.7f,1.0f };
}

void HandleSDLEvent(SDL_Event *event)
{
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			Screen::OnClick(&event->button);
			break;
		case SDL_MOUSEBUTTONUP:
			Screen::OnClick(&event->button);
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

namespace Theme {
	static const float BORDER_WIDTH = 2.0;
	
	void DrawHollowRect(const float size[2])
	{
		GLfloat vertices[] = { 0,0,
			0,size[1],
			size[0],size[1],
			size[0],0,
			BORDER_WIDTH,BORDER_WIDTH,
			BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,BORDER_WIDTH };
		GLubyte indices[] = {
			0,1,5,4, 0,4,7,3,
			3,7,6,2, 1,2,6,5 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glDrawElements(GL_QUADS, 16, GL_UNSIGNED_BYTE, indices);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void DrawIndent(const float size[2])
	{
		GLfloat vertices[] = { 0,0,
			0,size[1],
			size[0],size[1],
			size[0],0,
			BORDER_WIDTH,BORDER_WIDTH,
			BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,BORDER_WIDTH };
		GLubyte indices[] = {
			0,1,5,4, 0,4,7,3,
			3,7,6,2, 1,2,6,5,
			4,5,6,7 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glColor3fv(Color::bgShadow);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices);
		glColor3f(.6f,.6f,.6f);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+8);
		glColor3fv(Color::bg);
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+16);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void DrawOutdent(const float size[2])
	{
		GLfloat vertices[] = { 0,0,
			0,size[1],
			size[0],size[1],
			size[0],0,
			BORDER_WIDTH,BORDER_WIDTH,
			BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,
			size[0]-BORDER_WIDTH,BORDER_WIDTH };
		GLubyte indices[] = {
			0,1,5,4, 0,4,7,3,
			3,7,6,2, 1,2,6,5,
			4,5,6,7 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glColor3f(.6f,.6f,.6f);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices);
		glColor3fv(Color::bgShadow);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+8);
		glColor3fv(Color::bg);
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+16);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

}
