#include <cstdlib>
#include "SDL.h"
#include "ui/Context.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include <typeinfo>

static const int WIDTH  = 1024;
static const int HEIGHT = 768;

static bool click_handler(UI::Widget *w)
{
	printf("click: %p %s\n", w, typeid(*w).name());
	return true;
}

static bool move_handler(const UI::MouseMotionEvent &event, UI::Widget *w)
{
	printf("move: %p %s %f,%f\n", w, typeid(*w).name(), event.pos.x, event.pos.y);
	return true;
}

static bool over_handler(UI::Widget *w)
{
	printf("over: %p %s\n", w, typeid(*w).name());
	return true;
}

static bool out_handler(UI::Widget *w)
{
	printf("out: %p %s\n", w, typeid(*w).name());
	return true;
}

static void colour_change(float v, UI::Background *back, UI::Slider *r, UI::Slider *g, UI::Slider *b)
{
	back->SetColor(Color(r->GetValue(), g->GetValue(), b->GetValue()));
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

	UI::Context *c = new UI::Context(r, WIDTH, HEIGHT);

#if 0
	UI::Button *b1, *b2, *b3;
	c->SetInnerWidget(
		c->VBox()->PackEnd(UI::WidgetSet(
			c->Margin(10.0f)->SetInnerWidget(
				(b1 = c->Button())
			),
			c->Margin(10.0f)->SetInnerWidget(
				(b2 = c->Button())->SetInnerWidget(c->Image("icons/object_star_m.png"))
			),
            c->Margin(10.0f)->SetInnerWidget(
                (b3 = c->Button())->SetInnerWidget(c->Label("PEW PEW"))
            )), UI::Box::ChildAttrs(false, false)
		)
	);

	b1->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), b1));
//	b1->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), b1));
	b1->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b1));
	b1->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b1));
	b2->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), b2));
//	b2->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), b2));
	b2->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b2));
	b2->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b2));
	b3->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), b3));
//	b3->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), b3));
	b3->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b3));
	b3->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b3));
#endif

#if 0
	UI::Image *image;
	UI::Slider *slider;
	c->SetInnerWidget(
		c->Background(Color(0.4f, 0.2f, 0.4f, 1.0f))->SetInnerWidget(
			c->Margin(10.0f)->SetInnerWidget(
				c->Background(Color(0.1f, 0.4f, 0.4f, 1.0f))->SetInnerWidget(
					c->VBox()->PackEnd(UI::WidgetSet(
						c->HBox()->PackEnd(UI::WidgetSet(
							c->MultiLineText("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
							(image = c->Image("icons/object_star_g.png")),
							c->Image("icons/object_star_m.png")
						)),
						c->Background(Color(1.0f, 0.0f, 0.0f, 1.0f)),
						c->Background(Color(0.0f, 1.0f, 0.0f, 1.0f)),
						c->Background(Color(0.0f, 0.0f, 1.0f, 1.0f)),
						c->Image("icons/cpanel.png"),
						c->HBox(5.0f)->PackEnd(UI::WidgetSet(
							c->Button()->SetInnerWidget(c->Label("Load game")),
							c->Button()->SetInnerWidget(c->Label("Save game")),
							c->Button()->SetInnerWidget(c->Label("Win game"))
						), UI::Box::ChildAttrs(false, true))->PackEnd(
							(slider = c->HSlider()), UI::Box::ChildAttrs(true, true)
						)
					))
				)
			)
		)
	);

	image->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), image));
	image->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), image));
#endif

#if 0
	UI::Slider *red, *green, *blue;
	UI::Background *back;
	c->SetInnerWidget(
		c->VBox(5.0f)->PackEnd(UI::WidgetSet(
			c->HBox(5.0f)->PackEnd(c->Label("Red"), UI::Box::ChildAttrs(false))->PackEnd(red = c->HSlider()),
			c->HBox(5.0f)->PackEnd(c->Label("Green"), UI::Box::ChildAttrs(false))->PackEnd(green = c->HSlider()),
			c->HBox(5.0f)->PackEnd(c->Label("Blue"), UI::Box::ChildAttrs(false))->PackEnd(blue = c->HSlider())
		), UI::Box::ChildAttrs(false))->PackEnd(back = c->Background(Color()))
	);

	red->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
	green->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
	blue->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
#endif

	c->SetInnerWidget(
		c->Grid(UI::CellSpec(0.5f,0.5f), UI::CellSpec(0.5f,0.5f))
			->SetRow(0, UI::WidgetSet(
				c->MultiLineText("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
				c->Image("icons/object_star_g.png")
			))
			->SetRow(1, UI::WidgetSet(
				c->Image("icons/object_star_m.png"),
				c->Button()->SetInnerWidget(c->Label("Wear monocle"))
			))
	);

	c->Layout();

	while (1) {
		bool done = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				done = true;
			else
				c->DispatchSDLEvent(event);
		}

		if (done)
			break;

		c->Update();

		r->ClearScreen();
		c->Draw();
		r->SwapBuffers();

//		slider->SetValue(slider->GetValue() + 0.01);
	}

	delete c;
	delete r;

	SDL_Quit();

	exit(0);
}
