// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"
#include "graphics/Graphics.h"

namespace Gui {

namespace RawEvents {
	sigc::signal<void, MouseMotionEvent *> onMouseMotion;
	sigc::signal<void, MouseButtonEvent *> onMouseDown;
	sigc::signal<void, MouseButtonEvent *> onMouseUp;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
	sigc::signal<void, SDL_JoyAxisEvent *> onJoyAxisMotion;
	sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonDown;
	sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonUp;
	sigc::signal<void, SDL_JoyHatEvent *> onJoyHatMotion;
}

static Sint32 lastMouseX, lastMouseY;
void HandleSDLEvent(SDL_Event *event)
{
	PROFILE_SCOPED()
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			lastMouseX = event->button.x;
			lastMouseY = event->button.y;
			Screen::OnClick(&event->button);
			break;
		case SDL_MOUSEBUTTONUP:
			lastMouseX = event->button.x;
			lastMouseY = event->button.y;
			Screen::OnClick(&event->button);
			break;
		case SDL_MOUSEWHEEL: {
			// synthesizing an SDL1.2-style button event for mouse wheels
			SDL_MouseButtonEvent ev;
            ev.type = SDL_MOUSEBUTTONDOWN;
			ev.button = event->wheel.y > 0 ? MouseButtonEvent::BUTTON_WHEELUP : MouseButtonEvent::BUTTON_WHEELDOWN;
			ev.state = SDL_PRESSED;
			ev.x = lastMouseX;
			ev.y = lastMouseY;
			Screen::OnClick(&ev);
			break;
		 }
		case SDL_KEYDOWN:
			Screen::OnKeyDown(&event->key.keysym);
			RawEvents::onKeyDown.emit(&event->key);
			break;
		case SDL_KEYUP:
			Screen::OnKeyUp(&event->key.keysym);
			RawEvents::onKeyUp.emit(&event->key);
			break;
		case SDL_TEXTINPUT:
			Screen::OnTextInput(&event->text);
			break;
		case SDL_MOUSEMOTION:
			lastMouseX = event->motion.x;
			lastMouseY = event->motion.y;
			Screen::OnMouseMotion(&event->motion);
			break;
		case SDL_JOYAXISMOTION:
			RawEvents::onJoyAxisMotion(&event->jaxis);
			break;
		case SDL_JOYBUTTONUP:
			RawEvents::onJoyButtonUp(&event->jbutton);
			break;
		case SDL_JOYBUTTONDOWN:
			RawEvents::onJoyButtonDown(&event->jbutton);
			break;
		case SDL_JOYHATMOTION:
			RawEvents::onJoyHatMotion(&event->jhat);
			break;
	}
}

struct TimerSignal {
	Uint32 goTime;
	sigc::signal<void> sig;
};

static std::list<TimerSignal*> g_timeSignals;

sigc::connection AddTimer(Uint32 ms, sigc::slot<void> slot)
{
	TimerSignal *_s = new TimerSignal;
	_s->goTime = SDL_GetTicks() + ms;
	sigc::connection con = _s->sig.connect(slot);
	g_timeSignals.push_back(_s);
	return con;
}

void Draw()
{
	PROFILE_SCOPED()
	Uint32 t = SDL_GetTicks();
	// also abused like an update() function...
	for (std::list<TimerSignal*>::iterator i = g_timeSignals.begin(); i != g_timeSignals.end();) {
		if (t >= (*i)->goTime) {
			(*i)->sig.emit();
			delete *i;
			i = g_timeSignals.erase(i);
		} else {
			++i;
		}
	}
//	ExpireTimers(t);

	Screen::Draw();
}

void Init(Graphics::Renderer *renderer, int screen_width, int screen_height, int ui_width, int ui_height)
{
	Screen::Init(renderer, screen_width, screen_height, ui_width, ui_height);
}

void Uninit()
{
	std::list<TimerSignal*>::iterator i;
	for (i=g_timeSignals.begin(); i!=g_timeSignals.end(); ++i) delete *i;

	Screen::Uninit();
}

void MainLoopIteration()
{
	PROFILE_SCOPED()

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	r->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
	r->LoadIdentity();
	r->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
	r->LoadIdentity();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// handle events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		if (event.type == SDL_QUIT) {
			SDL_Quit();
			exit(0);
		}
	}

	SDL_ShowCursor(1);
	r->GetWindow()->SetGrab(false);
	Gui::Draw();
	r->GetWindow()->SwapBuffers();
}

namespace Theme {
	namespace Colors {
		const Color bg(.25f, .37f, .63f);
		const Color bgShadow(.08f, .12f, .21f);
		const Color tableHeading(.7f, .7f, 1.0f);
	}
	static const float BORDER_WIDTH = 2.0;

	void DrawRoundEdgedRect(const float size[2], float rad)
	{
		const int STEPS = 6;
		if (rad > 0.5f*std::min(size[0], size[1])) rad = 0.5f*std::min(size[0], size[1]);
		glBegin(GL_TRIANGLE_FAN);
			// top left
			// bottom left
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*0.5f*i/float(STEPS);
				glVertex2f(rad - rad*cos(ang), (size[1] - rad) + rad*sin(ang));
			}
			// bottom right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*0.5 + M_PI*0.5f*i/float(STEPS);
				glVertex2f(size[0] - rad - rad*cos(ang), (size[1] - rad) + rad*sin(ang));
			}
			// top right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI + M_PI*0.5f*i/float(STEPS);
				glVertex2f((size[0] - rad) - rad*cos(ang), rad + rad*sin(ang));
			}

			// top right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*1.5 + M_PI*0.5f*i/float(STEPS);
				glVertex2f(rad - rad*cos(ang), rad + rad*sin(ang));
			}
		glEnd();
	}

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
		glColor3fv(Colors::bgShadow);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices);
		glColor3f(.6f,.6f,.6f);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+8);
		glColor3fv(Colors::bg);
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
		glColor3fv(Colors::bgShadow);
		glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+8);
		glColor3fv(Colors::bg);
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+16);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

}
