#include <cstdlib>
#include "SDL.h"
#include "ui/Context.h"
#include "ui/Screen.h"
#include "ui/Background.h"
#include "ui/Box.h"
#include "ui/Image.h"
#include "ui/Label.h"
#include "ui/Margin.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include <typeinfo>

static const int WIDTH  = 1024;
static const int HEIGHT = 768;

static void click_handler(UI::Widget *w)
{
	printf("click: %p %s\n", w, typeid(*w).name());
}

static bool move_handler(const UI::MouseMotionEvent &event)
{
	printf("move: %f,%f\n", event.pos.x, event.pos.y);
	return true;
}

int main(int argc, char **argv)
{
	FileSystem::Init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "sdl init failed: %s\n", SDL_GetError());
		exit(-1);
	}

    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    switch (info->vfmt->BitsPerPixel) {
        case 16:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            break;
        case 24:
        case 32:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            break;
        default:
            fprintf(stderr, "invalid pixel depth: %d bpp\n", info->vfmt->BitsPerPixel);
            exit(-1);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	SDL_Surface *surface = SDL_SetVideoMode(WIDTH, HEIGHT, info->vfmt->BitsPerPixel, SDL_OPENGL);
	if (!surface) {
		fprintf(stderr, "sdl video mode init failed: %s\n", SDL_GetError());
		SDL_Quit();
		exit(-1);
	}

	SDL_WM_SetCaption("uitest", "uitest");

	Graphics::Renderer *r = Graphics::Init(WIDTH, HEIGHT, true);
	r->SetOrthographicProjection(0, WIDTH, HEIGHT, 0, -1, 1);
	r->SetTransform(matrix4x4f::Identity());
	r->SetClearColor(Color::BLACK);
	r->SetDepthTest(false);

	// XXX GL renderer enables lighting by default. if all draws use materials
	// that's ok, but for filled regions (ie Background) its not right. a
	// scissored version of Renderer::ClearScreen would be the most efficient,
	// but I'm not quite ready to do it yet.
	glDisable(GL_LIGHTING);

	UI::Context *c = new UI::Context(r);
	UI::Screen *screen = new UI::Screen(c, WIDTH, HEIGHT);

#if 0
	UI::Button *button;
	screen->SetInnerWidget(
		c->Margin(10.0f)->SetInnerWidget(
			(button = c->Button())
		)
	);

	button->onMouseUp.connect(sigc::ptr_fun(&click_handler));
	button->onMouseMove.connect(sigc::ptr_fun(&move_handler));
#endif

	UI::Image *image;
	screen->SetInnerWidget(
		c->Background(Color(0.4f, 0.2f, 0.4f, 1.0f))->SetInnerWidget(
			c->Margin(10.0f)->SetInnerWidget(
				c->Background(Color(0.1f, 0.4f, 0.4f, 1.0f))->SetInnerWidget(
					c->VBox()->PackEnd(UI::WidgetSet(
						c->HBox()->PackEnd(UI::WidgetSet(
							c->Label("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
							(image = c->Image("icons/object_star_g.png")),
							c->Image("icons/object_star_m.png")
						)),
						c->Background(Color(1.0f, 0.0f, 0.0f, 1.0f)),
						c->Background(Color(0.0f, 1.0f, 0.0f, 1.0f)),
						c->Background(Color(0.0f, 0.0f, 1.0f, 1.0f)),
						c->Image("icons/cpanel.png")
					))
				)
			)
		)
	);

	image->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), image));
	image->onMouseMove.connect(sigc::ptr_fun(&move_handler));

	while (1) {
		bool done = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				done = true;
			else
				screen->DispatchSDLEvent(event);
		}

		if (done)
			break;

		screen->Layout();
		screen->Update();

		r->ClearScreen();
		screen->Draw();
		r->SwapBuffers();
	}

	delete screen;
	delete c;
	delete r;

	SDL_Quit();

	exit(0);
}
