#include <cstdlib>
#include "SDL.h"
#include "ui/Context.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "Lua.h"
#include "PropertiedObject.h"
#include <typeinfo>

static const int WIDTH  = 1024;
static const int HEIGHT = 768;

#if 0
class Thing : public PropertiedObject {
public:
	Thing(LuaManager *lua) : PropertiedObject(lua) {
		Update();
	}

	void Update() {
		time_t t = time(0);
		Properties().Set("time", asctime(localtime(&t)));
	}
};

static bool toggle_disabled_handler(UI::Widget *w)
{
	w->IsDisabled() ? w->Enable() : w->Disable();
	printf("toggle disabled: %p %s now %s\n", w, typeid(*w).name(), w->IsDisabled() ? "DISABLED" : "ENABLED");
	return true;
}

static bool click_handler(UI::Widget *w)
{
	printf("click: %p %s\n", w, typeid(*w).name());
	return true;
}

static bool move_handler(const UI::MouseMotionEvent &event, UI::Widget *w)
{
	printf("move: %p %s %d,%d\n", w, typeid(*w).name(), event.pos.x, event.pos.y);
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

static void colour_change(float v, UI::ColorBackground *back, UI::Slider *r, UI::Slider *g, UI::Slider *b)
{
	back->SetColor(Color(r->GetValue(), g->GetValue(), b->GetValue()));
}

static void option_selected(unsigned int index, const std::string &option)
{
	printf("option selected: %d %s\n", index, option.c_str());
}

static void fill_label(float v, UI::Label *label)
{
	std::string s;
	for (int i = 0; i < int(v*100.0f); i++)
		s += "x";
	label->SetText(s);
}

static const char *options[] = { "foo", "bar", "baz", "qwop" };
static bool add_dropdown_option(UI::DropDown *dropdown)
{
	static int i = 0;
	dropdown->AddOption(options[i++]);
	i = i % 4;
	return true;
}

static bool clear_dropdown(UI::DropDown *dropdown)
{
	dropdown->Clear();
	return true;
}

static bool remove_widget(UI::VBox *box, UI::Widget *widget)
{
	box->Remove(widget);
	return true;
}

static bool remove_floating_widget(UI::Context *c, UI::Widget *widget)
{
	c->RemoveFloatingWidget(widget);
	return true;
}
#endif

int main(int argc, char **argv)
{
	FileSystem::Init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "sdl init failed: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_EnableUNICODE(1);

    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    switch (info->vfmt->BitsPerPixel) {
        case 16:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            break;
        case 24:
        case 32:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
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

	Graphics::Settings videoSettings;
	videoSettings.width = WIDTH;
	videoSettings.height = HEIGHT;
	videoSettings.fullscreen = false;
	videoSettings.shaders = false;
	videoSettings.requestedSamples = 0;
	videoSettings.vsync = false;
	videoSettings.useTextureCompression = false;
	Graphics::Renderer *r = Graphics::Init(videoSettings);

	Lua::Init();

	RefCountedPtr<UI::Context> c(new UI::Context(Lua::manager, r, WIDTH, HEIGHT, "English"));

#if 0
	UI::Gauge *gauge;
	c->SetInnerWidget(c->HBox()->PackEnd(gauge = c->Gauge()));
	gauge->SetWarningLevel(0.4f);
	gauge->SetCriticalLevel(0.2f);
	gauge->SetLevelAscending(false);
#endif

#if 0
	Thing thing(Lua::manager);

	UI::Label *l = c->Label("label");
	c->SetInnerWidget(l);

	l->Bind("text", &thing, "time");


	c->SetInnerWidget(
		c->VBox(10)->PackEnd(UI::WidgetSet(
			c->Background()->SetInnerWidget(
				c->HBox(5)->PackEnd(UI::WidgetSet(
					c->Expand(UI::Expand::HORIZONTAL)->SetInnerWidget(c->Label("right")),
					c->Icon("ArrowRight")
				))
			),
			c->Background()->SetInnerWidget(
				c->HBox(5)->PackEnd(UI::WidgetSet(
					c->Icon("ArrowLeft"),
					c->Expand(UI::Expand::HORIZONTAL)->SetInnerWidget(c->Label("left"))
				))
			)
		))
	);
#endif

#if 0
	UI::Button *toggle;
	UI::CheckBox *target;
	c->SetInnerWidget(
		c->HBox(10)->PackEnd(UI::WidgetSet(
			(toggle = c->Button()),
			(target = static_cast<UI::CheckBox*>(c->CheckBox()))
		))
	);

	toggle->onClick.connect(sigc::bind(sigc::ptr_fun(&toggle_disabled_handler), target));
	target->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), target));
	target->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), target));
	target->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), target));
#endif

#if 0
	c->SetInnerWidget(
		c->ColorBackground(Color(0.4f, 0.2f, 0.4f, 1.0f))->SetInnerWidget(
			c->HBox()->PackEnd(UI::WidgetSet(
				c->Icon("Agenda"),
				c->Icon("Bag"),
				c->Icon("Planet"),
				c->Icon("Satellite"),
				c->Icon("TrafficCone"),
				c->Label("Some text")->SetFont(UI::Widget::FONT_HEADING_XSMALL)
			))
		)
	);
#endif

#if 0
	c->SetInnerWidget(
		c->Margin(0)->SetInnerWidget(c->Gradient(Color(1.0f,0,0,1.0f), Color(0,0,1.0f,1.0f), UI::Gradient::HORIZONTAL))
	);
#endif

#if 0
	UI::Button *b1, *b2, *b3;
	c->SetInnerWidget(
		c->VBox()->PackEnd(UI::WidgetSet(
			c->Margin(10.0f)->SetInnerWidget(
				(b1 = c->Button())
			),
			c->Margin(10.0f)->SetInnerWidget(
				(b2 = c->Button())->SetInnerWidget(c->Image("icons/object_star_m.png", UI::Widget::PRESERVE_ASPECT))
			),
            c->Margin(10.0f)->SetInnerWidget(
                (b3 = c->Button())->SetInnerWidget(c->Label("PEW PEW"))
            )
		))
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
		c->ColorBackground(Color(0.4f, 0.2f, 0.4f, 1.0f))->SetInnerWidget(
			c->Margin(10.0f)->SetInnerWidget(
				c->ColorBackground(Color(0.1f, 0.4f, 0.4f, 1.0f))->SetInnerWidget(
					c->VBox()->PackEnd(UI::WidgetSet(
						c->Expand(UI::Expand::VERTICAL)->SetInnerWidget(
							c->HBox()->PackEnd(UI::WidgetSet(
								c->MultiLineText("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
								c->Icon("Twitter"),
								(image = c->Image("icons/object_star_g.png", UI::Widget::PRESERVE_ASPECT))
							))),
						c->Image("icons/cpanel.png", UI::Widget::EXPAND_WIDTH),
						c->HBox(5.0f)->PackEnd(UI::WidgetSet(
							c->Button()->SetInnerWidget(c->Label("Load game")),
							c->Button()->SetInnerWidget(c->Label("Save game")),
							c->Button()->SetInnerWidget(c->Label("Win game"))
						))->PackEnd(
							(slider = c->HSlider())
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
	UI::ColorBackground *back;
	c->SetInnerWidget(
		c->VBox(5.0f)->PackEnd(UI::WidgetSet(
			c->HBox(5.0f)->PackEnd(c->Label("Red"))->PackEnd(red = c->HSlider()),
			c->HBox(5.0f)->PackEnd(c->Label("Green"))->PackEnd(green = c->HSlider()),
			c->HBox(5.0f)->PackEnd(c->Label("Blue"))->PackEnd(blue = c->HSlider())
		))->PackEnd(c->Expand()->SetInnerWidget(back = c->ColorBackground(Color())))
	);

	red->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
	green->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
	blue->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&colour_change), back, red, green, blue));
#endif

#if 0
	c->SetInnerWidget(
		//c->Grid(UI::CellSpec(0.2f,0.8f), UI::CellSpec(0.7f,0.3f))
		c->Grid(3,3)
			->SetRow(0, UI::WidgetSet(
				c->MultiLineText("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."),
				c->Image("icons/object_star_g.png", UI::Widget::PRESERVE_ASPECT)
			))
			->SetRow(1, UI::WidgetSet(
				c->Image("icons/object_star_m.png", UI::Widget::PRESERVE_ASPECT),
				c->Button()->SetInnerWidget(c->Label("Wear monocle"))
			))
	);
#endif

#if 0
	UI::ColorBackground *back[4];
	UI::Button *button[5];
	c->SetInnerWidget(
		c->Grid(2,2)
			->SetRow(0, UI::WidgetSet(
				(back[0] = c->ColorBackground(Color(0.8f,0.2f,0.2f))),
				(back[1] = c->ColorBackground(Color(0.2f,0.8f,0.2f)))))
			->SetRow(1, UI::WidgetSet(
				(back[2] = c->ColorBackground(Color(0.2f,0.2f,0.8f))),
				(back[3] = c->ColorBackground(Color(0.8f,0.8f,0.2f)))))
	);
	c->AddFloatingWidget(
		(button[0] = c->Button())->SetInnerWidget(c->Image("icons/object_star_m.png")), UI::Point(472, 344), UI::Point(80)
	)->AddFloatingWidget(
		(button[1] = c->Button())->SetInnerWidget(c->Image("icons/object_star_a.png")), UI::Point(216, 344), UI::Point(80)
	)->AddFloatingWidget(
		(button[2] = c->Button())->SetInnerWidget(c->Image("icons/object_star_f.png")), UI::Point(728, 344), UI::Point(80)
	)->AddFloatingWidget(
		(button[3] = c->Button())->SetInnerWidget(c->Image("icons/object_star_g.png")), UI::Point(472, 152), UI::Point(80)
	)->AddFloatingWidget(
		(button[4] = c->Button())->SetInnerWidget(c->Image("icons/object_star_k.png")), UI::Point(472, 536), UI::Point(80)
	);

	for (int i = 0; i < 4; i++) {
		back[i]->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), back[i]));
//		back[i]->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), back[i]));
		back[i]->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), back[i]));
		back[i]->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), back[i]));
	}
	for (int i = 0; i < 5; i++) {
		button[i]->onClick.connect(sigc::bind(sigc::ptr_fun(&click_handler), button[i]));
//		button[i]->onMouseMove.connect(sigc::bind(sigc::ptr_fun(&move_handler), button[i]));
		button[i]->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), button[i]));
		button[i]->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), button[i]));
	}
    c->AddShortcut(UI::KeySym(SDLK_a, KMOD_LCTRL), button[0]);
#endif

#if 0
	UI::DropDown *dropdown;
	UI::List *list;
	c->SetInnerWidget(
		c->VBox()->PackEnd(UI::WidgetSet(
			c->HBox()->PackEnd(
				(dropdown = c->DropDown()
					->AddOption("watermelon")
					->AddOption("banana")
					->AddOption("ox tongue")
				)
			),
			c->HBox()->PackEnd(UI::WidgetSet(
				c->CheckBox(),
				c->Label("Please add me to your mailing list")
			)),
			c->Margin(10.0f)->SetInnerWidget(
				(list = c->List()
					->AddOption("foo")
					->AddOption("bar")
					->AddOption("baz")
					->AddOption("qwop"))
			)
		))
	);
	dropdown->SetFont(UI::Widget::FONT_HEADING_XLARGE);
	dropdown->onOptionSelected.connect(sigc::ptr_fun(&option_selected));
	list->onOptionSelected.connect(sigc::ptr_fun(&option_selected));
#endif

#if 0
	c->SetInnerWidget(
		c->Scroller()->SetInnerWidget(
			c->MultiLineText(
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus consectetur risus augue. Aenean porttitor enim dolor, vitae iaculis mi. Etiam a nibh at massa dictum blandit. Etiam sed varius quam. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Praesent facilisis tortor nisi. Maecenas ut enim nulla, pharetra elementum dolor. Vivamus condimentum semper magna laoreet gravida. Proin vulputate odio eget metus tristique tristique. Donec viverra augue quis velit lacinia vel dapibus diam volutpat. Fusce laoreet dui sit amet magna sagittis porttitor. Fusce sodales nulla id eros vehicula at pulvinar nisl facilisis. In ut neque lorem, ut vehicula tellus. Donec a posuere quam.\n\n"
	"Praesent congue tempus est, non sodales magna tincidunt sit amet. In quis nulla metus. Donec vel viverra nulla. Curabitur sapien ipsum, accumsan eget lobortis nec, pretium quis enim. In et condimentum justo. Curabitur dictum lorem et arcu tincidunt mattis. Suspendisse sollicitudin hendrerit nisi, id vehicula purus tempus ut. Morbi sit amet neque mauris, tincidunt aliquet purus. Aenean vitae massa eu tortor viverra lobortis.\n\n"
	"Vestibulum lorem dui, porttitor vitae convallis eu, fermentum dictum arcu. Proin eget dolor metus. Donec hendrerit gravida augue, sed accumsan leo fringilla nec. Curabitur mollis facilisis tortor, sed lobortis augue sagittis eget. Pellentesque faucibus suscipit placerat. Nam non sem justo, vitae bibendum diam. Quisque ut malesuada est. Nulla nibh elit, molestie eu aliquam id, ullamcorper a ipsum. Quisque eget euismod massa. Quisque commodo molestie velit, et ornare nulla convallis vel. Sed molestie lacus in augue bibendum dapibus. Cras sit amet sagittis nunc. Etiam leo ante, faucibus vel bibendum et, viverra id dolor. Vestibulum libero elit, egestas egestas elementum vitae, aliquam vel nisl. Duis ligula turpis, consequat et sagittis sit amet, porttitor non lorem.\n\n"
	"Suspendisse et sapien at ante vestibulum tempor. Suspendisse id nisl ac urna interdum sagittis nec at eros. Aliquam in faucibus augue. Sed rutrum laoreet sem, quis pellentesque lorem bibendum sit amet. Nulla molestie mauris ac nunc convallis mollis. Integer ullamcorper, ligula sit amet ultrices sagittis, tortor purus scelerisque risus, at imperdiet leo nisi sit amet neque. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Nunc mattis arcu non velit accumsan id scelerisque orci consectetur. Ut a lectus tortor, at aliquet sem. Maecenas et tortor non lectus euismod porta. Integer quis nibh odio, sit amet malesuada dolor. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Fusce vestibulum rhoncus quam, in congue dolor tristique pellentesque.\n\n"
	"Proin quam nisi, condimentum at pulvinar id, ultricies et neque. Duis rutrum imperdiet massa, quis blandit augue dapibus et. Proin arcu eros, pharetra non sollicitudin at, congue ac sapien. In vitae ligula at est laoreet accumsan et et erat. Vivamus sit amet scelerisque ligula. In quis velit sed justo hendrerit eleifend. Donec mi felis, malesuada imperdiet tincidunt ac, vehicula et turpis. Pellentesque a dolor felis, quis ultrices magna. Integer nec nisi ut nulla pharetra condimentum et at massa. Vestibulum lobortis commodo purus, faucibus ullamcorper justo laoreet sed. Suspendisse nisi mauris, ornare sit amet mollis vitae, luctus nec tortor. Ut auctor mollis vehicula. Donec rutrum arcu mauris. Sed id egestas erat. Vivamus facilisis volutpat lectus quis scelerisque.\n\n"
	"Mauris id mi eget ipsum placerat aliquet. Vestibulum sed lacus nec felis malesuada ornare nec in nisi. Phasellus viverra felis a dolor scelerisque gravida a non massa. Nunc non arcu dolor, eu molestie mauris. In dui quam, malesuada nec condimentum et, interdum eget risus. Aenean dolor nisi, fringilla non rhoncus laoreet, vulputate ut dolor. Aliquam quis augue ligula.\n\n"
	"Fusce semper, lectus non lacinia condimentum, tellus lectus scelerisque lacus, eget sollicitudin tortor velit non turpis. Vivamus rutrum cursus nulla, non tincidunt arcu dapibus vitae. Mauris elementum rhoncus purus, at tristique tortor venenatis vitae. Sed a est purus. Etiam in lorem sed eros molestie dapibus. Aenean nunc lectus, ornare ut condimentum et, mattis non purus. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Suspendisse sodales dolor nec odio iaculis vitae commodo diam scelerisque. Sed ac lectus in mi dictum aliquam elementum vitae dolor. Nulla lectus neque, pulvinar ut suscipit quis, tristique vel diam. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce et risus nunc. Integer ac nibh ut ligula tempus fringilla vitae vel orci. Quisque luctus elit at est semper sagittis. Mauris nec eros eget mauris gravida tincidunt et ac eros.\n\n"
	"Aliquam at tortor sed elit ultricies vestibulum vel quis mi. Proin consectetur, purus sed scelerisque ornare, erat augue vulputate dui, sed congue tortor lectus id elit. Nunc tempor sagittis dolor. Vestibulum interdum malesuada nisi sit amet tempus. Fusce ullamcorper, tortor quis placerat auctor, lectus lacus pellentesque elit, a tristique sem tellus feugiat lacus. Nam nunc dui, vehicula eget fringilla eget, venenatis vitae nibh. Pellentesque ultrices interdum diam, vitae varius purus convallis eu. Praesent ac orci id libero hendrerit faucibus eu at massa. Integer id odio mi. In urna massa, eleifend a placerat et, ornare quis est. Aliquam metus tellus, rhoncus et imperdiet in, imperdiet sit amet erat. Quisque dapibus, dui ac aliquet ultricies, arcu erat lacinia augue, ac suscipit nunc diam ac lorem. Fusce ac lacus molestie felis cursus luctus sed convallis lacus. Nullam accumsan, tortor scelerisque suscipit fermentum, lorem lacus feugiat urna, non mollis eros enim non odio. Fusce nec eros turpis. Quisque et nisl lorem.\n\n"
	"Ut auctor bibendum augue, vestibulum consectetur magna euismod at. In sagittis vulputate nulla, vel scelerisque velit sollicitudin sit amet. Morbi tempus egestas nisl, et sagittis risus venenatis non. Sed ut lacus est. Phasellus mollis rutrum neque, in euismod nisi viverra non. Sed id nibh in dolor egestas consectetur semper eu dui. Quisque malesuada augue quis nisi consequat luctus. Quisque velit urna, euismod vestibulum dapibus vitae, volutpat nec libero. Curabitur sed hendrerit nibh. Pellentesque non mi ipsum. Praesent lectus tellus, faucibus sed blandit vitae, aliquam sed erat. Quisque vel adipiscing sapien. Fusce iaculis arcu commodo est consectetur non pretium risus adipiscing. Vivamus commodo nulla a felis rhoncus lacinia. Vestibulum diam eros, pretium eget fermentum nec, volutpat euismod erat.\n\n"
	"Nullam vel nibh ac tellus congue tristique. Curabitur volutpat, odio nec ullamcorper semper, lectus nibh lacinia ante, in aliquet nisi nulla imperdiet diam. Integer diam nulla, euismod in condimentum at, accumsan et est. Integer ligula neque, porta eget auctor adipiscing, tristique molestie tortor. Donec sit amet rutrum nisl. Proin venenatis lorem sed nibh feugiat vehicula. Integer eu justo nisl, in sollicitudin sapien.\n\n"
	"Donec elementum elit eu tortor pharetra pellentesque. Maecenas congue iaculis ultricies. Cras bibendum orci vel turpis consequat eget dignissim purus varius. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi vel justo non est consectetur dictum egestas non velit. Phasellus lobortis fringilla interdum. Cras mattis, est sed varius malesuada, lacus leo facilisis massa, ac egestas arcu lectus vitae magna. Nam a facilisis justo. Nullam justo libero, eleifend eget facilisis nec, scelerisque in metus. Morbi nec aliquam arcu. Mauris varius semper enim, nec vestibulum mi porta eu. Suspendisse ornare posuere eros eu placerat. Nunc in purus velit, ut sagittis ipsum.\n\n"
	"Proin auctor dapibus erat, vel sodales tellus sodales sit amet. Proin rutrum dui porttitor velit iaculis aliquet. Sed porttitor erat ac lacus accumsan tincidunt. Praesent tempus rhoncus augue dignissim porta. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Aliquam erat volutpat. Mauris facilisis sodales vestibulum.\n\n"
	"Phasellus accumsan, ante eu ornare lobortis, odio justo elementum felis, vitae vestibulum libero sem eget libero. Nulla justo orci, adipiscing ut tempor eu, tempus et lacus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Vivamus turpis odio, commodo quis dapibus eu, volutpat nec augue. Nam eget erat vel ligula laoreet pharetra sed eu odio. Sed condimentum dignissim velit ut commodo. Nunc vel metus purus, a tempus eros. Sed condimentum sollicitudin augue et pretium.\n\n"
	"In ornare mollis purus consectetur sagittis. Aliquam auctor, felis et vestibulum euismod, augue massa tincidunt purus, ut pellentesque diam ante eget est. Curabitur mattis lacus sed enim porta dapibus. Aenean auctor dolor quis felis accumsan hendrerit. Suspendisse vel velit sit amet ante posuere auctor. Mauris vitae erat vitae urna porttitor consectetur. Maecenas blandit massa sed nisl pulvinar et molestie mi ultricies. Quisque rhoncus enim vitae dui fermentum nec consequat nulla adipiscing. Aliquam feugiat dolor id diam volutpat gravida. Ut tincidunt scelerisque arcu, vel faucibus mauris viverra id.\n\n"
	"Nulla facilisi. Sed dictum massa dignissim elit ultricies sagittis. Mauris metus nunc, vehicula nec pharetra at, tristique id augue. Phasellus pellentesque erat in metus sagittis eu tempor felis hendrerit. Phasellus varius malesuada vehicula. Nunc rutrum posuere velit, convallis laoreet nibh pellentesque eget. In ut leo odio, sed fermentum lorem. Sed malesuada mi et nunc tempor in pellentesque magna facilisis. Donec at enim a neque laoreet commodo.\n\n"
	"Pellentesque imperdiet ligula rutrum dolor imperdiet rutrum. Integer sapien quam, sagittis vitae ultricies et, commodo suscipit leo. Cras laoreet suscipit adipiscing. Donec felis ante, pharetra a dignissim ac, hendrerit eu augue. Nulla facilisi. Cras diam arcu, congue at egestas eu, rutrum quis nulla. Sed malesuada velit vel nulla blandit pulvinar et ac quam. Nullam eleifend dapibus fringilla. Aliquam mattis ante vel justo lobortis non fringilla mi mollis.\n\n"
	"Duis ultricies arcu non urna vulputate in volutpat erat venenatis. Duis pulvinar, tortor in gravida lobortis, felis risus cursus tortor, eget pulvinar erat purus ut leo. Vivamus vel diam augue, quis condimentum nisl. Sed sit amet felis in lectus ultrices ullamcorper. Duis elit ligula, dignissim quis venenatis eget, semper eu felis. Donec sollicitudin nibh ac ante sagittis auctor. Nunc nisi massa, eleifend eu sodales id, blandit sed turpis. Etiam cursus feugiat diam, vel ultrices urna porta quis. Pellentesque massa metus, aliquam ut dictum id, vulputate nec elit. Phasellus sed felis sapien, non lobortis urna. Nam in lacus lectus. Vivamus blandit neque eu turpis dapibus ultricies interdum ligula rutrum. Donec dapibus accumsan consectetur. Phasellus sollicitudin mi id ante convallis non tincidunt magna laoreet.\n\n"
	"Nulla tellus augue, porta eget molestie eu, pellentesque ac mauris. Morbi est arcu, vehicula viverra venenatis a, pretium tristique libero. Nam consectetur dolor vulputate elit convallis vitae imperdiet neque adipiscing. Nullam ut quam sit amet sem tincidunt mattis. Aenean at lectus sed dui vestibulum ornare. Aliquam consectetur, libero eu semper rhoncus, neque lorem dictum sem, vel tincidunt mauris nunc vitae ante. Nunc et venenatis elit. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Proin posuere sapien sit amet nulla ultricies tincidunt.\n\n"
	"Mauris nec euismod erat. Vivamus vel nisl eu tortor dapibus vulputate at eget justo. Quisque libero augue, dictum semper scelerisque eu, semper a velit. Ut odio lectus, rhoncus vel congue sit amet, cursus ac nibh. Donec ultrices, erat vitae ultrices interdum, quam lacus scelerisque purus, eget vehicula nunc risus ac nisl. Nullam malesuada scelerisque purus ac tempus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ut tortor sed metus hendrerit elementum vel vitae nisl. Cras eu neque est, eu ullamcorper dui. In hac habitasse platea dictumst. Nullam non odio a sem cursus accumsan venenatis ac metus. Nunc commodo pulvinar sem, sit amet viverra mi rutrum id. Morbi at lorem vitae nunc dapibus sollicitudin vitae nec libero. Donec imperdiet ultrices ullamcorper.\n\n"
	"Quisque vitae libero massa. Mauris pulvinar ultrices rutrum. Quisque non nulla quis mi semper egestas sit amet sed arcu. Nam id sem ante. Quisque consectetur fermentum urna, a scelerisque nunc pulvinar vitae. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Pellentesque neque ipsum, ullamcorper at sagittis at, imperdiet lacinia lacus. Vestibulum venenatis dapibus leo, vel egestas risus mollis vel. Ut tincidunt faucibus diam, id varius purus lobortis non. Etiam elementum tincidunt tortor, id dictum erat mollis scelerisque. Fusce ac lorem diam, sit amet interdum nisi."
			)
		)
	);
#endif

#if 0
	UI::Label *label;
	UI::Slider *slider;
	c->SetInnerWidget(
		c->HBox(5.0f)->PackEnd(label = c->Label(""))->PackEnd(slider = c->HSlider()),
	);
	slider->onValueChanged.connect(sigc::bind(sigc::ptr_fun(&fill_label), label));
#endif

#if 0
	UI::DropDown *dropdown;
	UI::Button *add, *clear;
	c->SetInnerWidget(
		c->Margin(10.0f)->SetInnerWidget(
			c->VBox()->PackEnd(UI::WidgetSet(
				c->HBox()->PackEnd(UI::WidgetSet(
					(add = c->Button()),
					c->Label("Add")
				)),
				c->HBox()->PackEnd(UI::WidgetSet(
					(clear = c->Button()),
					c->Label("Clear")
				)),
				(dropdown = c->DropDown())
			))
		)
	);
	add->onClick.connect(sigc::bind(sigc::ptr_fun(&add_dropdown_option), dropdown));
	clear->onClick.connect(sigc::bind(sigc::ptr_fun(&clear_dropdown), dropdown));
#endif

#if 0
	c->SetInnerWidget(
		c->VBox()->PackEnd(UI::WidgetSet(
			c->Label("through three cheese trees three freezy fleas flew")->SetFont(UI::Widget::FONT_XSMALL),
			c->Label("through three cheese trees three freezy fleas flew")->SetFont(UI::Widget::FONT_SMALL),
			c->Label("through three cheese trees three freezy fleas flew")->SetFont(UI::Widget::FONT_NORMAL),
			c->Label("through three cheese trees three freezy fleas flew")->SetFont(UI::Widget::FONT_LARGE),
			c->Label("through three cheese trees three freezy fleas flew")->SetFont(UI::Widget::FONT_XLARGE)
		))
	);
#endif

#if 0
	UI::VBox *box;
	UI::Button *b1, *b2, *b3, *b4;
	c->SetInnerWidget(
		(box = c->VBox())->PackEnd(UI::WidgetSet(
			(b1 = c->Button())->SetInnerWidget(c->Label("remove other")),
			(b2 = c->Button())->SetInnerWidget(c->Label("other")),
			(b3 = c->Button())->SetInnerWidget(c->Label("remove me"))
		))
	);

	c->AddFloatingWidget( (b4 = c->Button())->SetInnerWidget(c->Label("remove me (float)")), 300, 300);

	b1->onClick.connect(sigc::bind(sigc::ptr_fun(&remove_widget), box, b2));
	b2->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b2));
	b2->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b2));
	b3->onClick.connect(sigc::bind(sigc::ptr_fun(&remove_widget), box, b3));
	b3->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b3));
	b3->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b3));
	b4->onClick.connect(sigc::bind(sigc::ptr_fun(&remove_floating_widget), c.Get(), b4));
	b4->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b4));
	b4->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b4));

	c->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), c.Get()));
	c->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), c.Get()));
#endif

#if 0
	c->SetInnerWidget(
		c->Grid(3,3)
			->SetRow(0, UI::WidgetSet(
				c->ColorBackground(Color(0.8f,0.2f,0.2f))->SetInnerWidget(c->Align(UI::Align::TOP_LEFT)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.2f,0.8f,0.2f))->SetInnerWidget(c->Align(UI::Align::TOP)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.2f,0.2f,0.8f))->SetInnerWidget(c->Align(UI::Align::TOP_RIGHT)->SetInnerWidget(c->Image("icons/object_star_m.png")))
			))->SetRow(1, UI::WidgetSet(
				c->ColorBackground(Color(0.8f,0.8f,0.2f))->SetInnerWidget(c->Align(UI::Align::LEFT)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.8f,0.2f,0.8f))->SetInnerWidget(c->Align(UI::Align::MIDDLE)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.2f,0.8f,0.8f))->SetInnerWidget(c->Align(UI::Align::RIGHT)->SetInnerWidget(c->Image("icons/object_star_m.png")))
			))->SetRow(2, UI::WidgetSet(
				c->ColorBackground(Color(0.5f,0.2f,0.8f))->SetInnerWidget(c->Align(UI::Align::BOTTOM_LEFT)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.8f,0.5f,0.2f))->SetInnerWidget(c->Align(UI::Align::BOTTOM)->SetInnerWidget(c->Image("icons/object_star_m.png"))),
				c->ColorBackground(Color(0.2f,0.8f,0.5f))->SetInnerWidget(c->Align(UI::Align::BOTTOM_RIGHT)->SetInnerWidget(c->Image("icons/object_star_m.png")))
			))
	);
#endif

#if 0
    c->SetInnerWidget(
        c->VBox()->PackEnd(
            c->Grid(2,2)
                ->SetRow(0, UI::WidgetSet(c->Label("one"), c->Label("two")))
                ->SetRow(1, UI::WidgetSet(c->Label("three"), c->Label("four")))
        )->PackEnd(c->ColorBackground(Color(0.8f,0.2f,0.2f)))
    );
#endif

#if 0
	UI::MultiLineText *text;
	c->SetInnerWidget(
		c->Scroller()->SetInnerWidget(
			(text = c->MultiLineText(""))
		)
	);
#endif

#if 0
	UI::VBox *box;
	UI::Button *b1, *b2, *b3, *b4;
	c->SetInnerWidget(
		(box = c->VBox())->PackEnd(UI::WidgetSet(
			(b1 = c->Button())->SetInnerWidget(c->Label("1")),
			(b2 = c->Button())->SetInnerWidget(c->Label("2")),
			(b3 = c->Button())->SetInnerWidget(c->Label("3")),
			(b4 = c->Button())->SetInnerWidget(c->Label("4")),
			c->TextEntry()
		))
	);
	b1->onClick.connect(sigc::bind(sigc::ptr_fun(click_handler), b1));
	b2->onClick.connect(sigc::bind(sigc::ptr_fun(click_handler), b2));
	b3->onClick.connect(sigc::bind(sigc::ptr_fun(click_handler), b3));
	b4->onClick.connect(sigc::bind(sigc::ptr_fun(click_handler), b4));
	b1->AddShortcut(UI::KeySym::FromString("shift+1"));
	b2->AddShortcut(UI::KeySym::FromString("ctrl+2"));
	b3->AddShortcut(UI::KeySym::FromString("alt+3"));
	b4->AddShortcut(UI::KeySym::FromString("ctrl+shift+4"));
#endif

	UI::Table *table;
	table = c->Table();
	table->SetFont(UI::Widget::FONT_LARGE);
	table->SetHeadingRow(UI::WidgetSet(
		c->Label("three"),
		c->Label("ten"),
		c->Label("twenty")
	));
	for (char ch = 'a'; ch <= 'z'; ch++) {
		static char buf[32];
		memset(buf, ch, sizeof(buf));
		UI::Label *l1, *l2, *l3;
		buf[20] = '\0';
		l3 = c->Label(buf);
		buf[10] = '\0';
		l2 = c->Label(buf);
		buf[3] = '\0';
		l1 = c->Label(buf);
		table->AddRow(UI::WidgetSet(l1, l2, l3));
	}
	c->SetInnerWidget(c->Grid(2,1)->SetCell(0,0,table));

	//int count = 0;

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

//		thing.Update();

//		slider->SetValue(slider->GetValue() + 0.01);
//		gauge->SetValue(gauge->GetValue() + 0.001);

#if 0
		if (++count == 400) {
			UI::Background *b;
			c->AddFloatingWidget((b = c->Background())->SetInnerWidget(c->Margin(100.0f)), 100.0f, 100.0f);
			b->onMouseOver.connect(sigc::bind(sigc::ptr_fun(&over_handler), b));
			b->onMouseOut.connect(sigc::bind(sigc::ptr_fun(&out_handler), b));
			c->Layout();
		}
		else if (count < 400 && count % 10 == 0)
			printf("%d\n", count);
#endif

#if 0
		if (++count % 10 == 0)
			text->AppendText("line\n");
		if (++count % 100 == 0)
			toggle_disabled_handler(target);
#endif

	}

	c.Reset();
	Lua::Uninit();
	delete r;

	SDL_Quit();

	exit(0);
}
