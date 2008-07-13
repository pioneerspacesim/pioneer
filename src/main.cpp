#include "libs.h"
#include "Pi.h"
#include "Gui.h"
#include "glfreetype.h"
#include "objimport.h"
#include "Player.h"
#include "Space.h"
#include "Planet.h"
#include "Star.h"
#include "Frame.h"
#include "ShipCpanel.h"
#include "SectorView.h"
#include "SystemView.h"
#include "SystemInfoView.h"
#include "WorldView.h"
#include "StarSystem.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "InfoView.h"

float Pi::timeAccel = 1.0f;
int Pi::scrWidth;
int Pi::scrHeight;
float Pi::scrAspect;
SDL_Surface *Pi::scrSurface;
sigc::signal<void, SDL_keysym*> Pi::onKeyPress;
sigc::signal<void, SDL_keysym*> Pi::onKeyRelease;
sigc::signal<void, int, int, int> Pi::onMouseButtonUp;
sigc::signal<void, int, int, int> Pi::onMouseButtonDown;
char Pi::keyState[SDLK_LAST];
char Pi::mouseButton[5];
int Pi::mouseMotion[2];
enum Pi::CamType Pi::cam_type;
enum Pi::MapView Pi::map_view;
Player *Pi::player;
View *Pi::current_view;
WorldView *Pi::world_view;
SpaceStationView *Pi::spaceStationView;
InfoView *Pi::infoView;
SectorView *Pi::sector_view;
SystemView *Pi::system_view;
SystemInfoView *Pi::system_info_view;
ShipCpanel *Pi::cpan;
StarSystem *Pi::selected_system;
MTRand Pi::rng;
double Pi::gameTime;
float Pi::frameTime;
GLUquadric *Pi::gluQuadric;
systemloc_t Pi::playerLoc;

void Pi::Init(IniConfig &config)
{
	int width = config.Int("ScrWidth");
	int height = config.Int("ScrHeight");
	const SDL_VideoInfo *info = NULL;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
		exit(-1);
	}

	info = SDL_GetVideoInfo();

	switch (config.Int("ScrDepth")) {
		case 16:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			break;
		case 32:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			break;
		default:
			fprintf(stderr, "Fatal error. Invalid screen depth in config.ini.\n");
			Pi::Quit();
	} 
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	Uint32 flags = SDL_OPENGL;
	if (config.Int("StartFullscreen")) flags |= SDL_FULLSCREEN;

	if ((Pi::scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags)) == 0) {
		// fall back on 16-bit depth buffer...
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer.\n", SDL_GetError());
		if ((Pi::scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags)) == 0) {
			fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
			exit(-1);
		}
	}

	Pi::scrWidth = width;
	Pi::scrHeight = height;
	Pi::scrAspect = width / (float)height;

	InitOpenGL();

	dInitODE();
	GLFTInit();
	Space::Init();
}

void Pi::InitOpenGL()
{
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	glClearColor(0,0,0,0);
	glViewport(0, 0, scrWidth, scrHeight);

	gluQuadric = gluNewQuadric ();
}

void Pi::Quit()
{
	SDL_Quit();
	exit(0);
}

void Pi::SetCamType(enum CamType c)
{
	cam_type = c;
	map_view = MAP_NOMAP;
	SetView(world_view);
}

void Pi::SetMapView(enum MapView v)
{
	map_view = v;
	if (v == MAP_SECTOR)
		SetView(sector_view);
	else
		SetView(system_view);
}

void Pi::SetView(View *v)
{
	if (current_view) current_view->HideAll();
	current_view = v;
	current_view->ShowAll();
}

void Pi::HandleEvents()
{
	SDL_Event event;

	Pi::mouseMotion[0] = Pi::mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q) Pi::Quit();
				if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(Pi::scrSurface);
				Pi::keyState[event.key.keysym.sym] = 1;
				Pi::onKeyPress.emit(&event.key.keysym);
				break;
			case SDL_KEYUP:
				Pi::keyState[event.key.keysym.sym] = 0;
				Pi::onKeyRelease.emit(&event.key.keysym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				Pi::mouseButton[event.button.button] = 1;
				Pi::onMouseButtonDown.emit(event.button.button,
						event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				Pi::mouseButton[event.button.button] = 0;
				Pi::onMouseButtonUp.emit(event.button.button,
						event.button.x, event.button.y);
				break;
			case SDL_MOUSEMOTION:
				Pi::mouseMotion[0] += event.motion.xrel;
				Pi::mouseMotion[1] += event.motion.yrel;
		//		SDL_GetRelativeMouseState(&Pi::mouseMotion[0], &Pi::mouseMotion[1]);
				break;
			case SDL_QUIT:
				Pi::Quit();
				break;
		}
	}
}

void Pi::MainLoop()
{
	Frame *earth_frame = new Frame(Space::GetRootFrame(), "Earth");
	earth_frame->SetPosition(vector3d(149598000000.0,0,0));
	earth_frame->SetRadius(2*380000000); // 2 moon orbital radii

	player = new Player(ShipType::SWANKY);
	player->SetLabel("me");
	player->SetFrame(earth_frame);
	player->SetPosition(vector3d(0,0,8000000.0));
	Space::AddBody(player);

	for (int i=0; i<4; i++) {
		Ship *body = new Ship(ShipType::LADYBIRD);
		char buf[64];
		snprintf(buf,sizeof(buf),"X%c-0%02d", 'A'+i, i);
		body->SetLabel(buf);
		body->SetFrame(earth_frame);
		body->SetPosition(vector3d(i*2000,0,8000400));
		Space::AddBody(body);
	}
		
	{
		SpaceStation *body = new SpaceStation();
		body->SetLabel("Poemi-chan's Folly");
		body->SetFrame(earth_frame);
		body->SetPosition(vector3d(0,0,7998000));
		Space::AddBody(body);
	}

	Planet *planet = new Planet(StarSystem::SBody::SUBTYPE_PLANET_INDIGENOUS_LIFE);
	planet->SetLabel("Earth");
	planet->SetPosition(vector3d(0,0,0));
	planet->SetFrame(earth_frame);
	Space::AddBody(planet);

	Frame *moon_frame = new Frame(earth_frame, "Moon");
	moon_frame->SetPosition(vector3d(0,-380000000.0,0));
	moon_frame->SetRadius(10*1738140.0); //10 moon radii
	Planet *moon = new Planet(StarSystem::SBody::SUBTYPE_PLANET_DWARF);
	moon->SetLabel("Moon");
	moon->SetPosition(vector3d(0,0,0));
	moon->SetRadius(1738140.0);
	moon->SetFrame(moon_frame);
	Space::AddBody(moon);

	Star *sol = new Star(StarSystem::SBody::SUBTYPE_STAR_G);
	sol->SetLabel("Sol");
	sol->SetRadius(6.955e8);
	sol->SetPosition(vector3d(0,0,0));
	sol->SetFrame(Space::GetRootFrame());
	Space::AddBody(sol);

	Gui::Init(scrWidth, scrHeight, 640, 480);

	cpan = new ShipCpanel();
	cpan->ShowAll();

	sector_view = new SectorView();
	system_view = new SystemView();
	system_info_view = new SystemInfoView();
	world_view = new WorldView();
	spaceStationView = new SpaceStationView();
	infoView = new InfoView();

	SetView(world_view);

	Uint32 last_stats = SDL_GetTicks();
	int frame_stat = 0;
	char fps_readout[32];
	Uint32 time_before_frame = SDL_GetTicks();

	for (;;) {
		frame_stat++;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		current_view->Draw3D();
		// XXX HandleEvents at the moment must be after view->Draw3D and before
		// Gui::Draw so that labels drawn to screen can have mouse events correctly
		// detected. Gui::Draw wipes memory of label positions.
		Pi::HandleEvents();
		// hide cursor for ship control.
		if (Pi::MouseButtonState(3)) {
			SDL_ShowCursor(0);
			SDL_WM_GrabInput(SDL_GRAB_ON);
		} else {
			SDL_ShowCursor(1);
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		}

		Gui::Draw();
#ifdef DEBUG
		{
			Gui::Screen::EnterOrtho();
			glColor3f(1,1,1);
			glTranslatef(0, Gui::Screen::GetHeight()-20, 0);
			Gui::Screen::RenderString(fps_readout);
			Gui::Screen::LeaveOrtho();
		}
#endif /* DEBUG */

		glFlush();
		SDL_GL_SwapBuffers();
		//if (glGetError()) printf ("GL: %s\n", gluErrorString (glGetError ()));
		
		Pi::frameTime = 0.001*(SDL_GetTicks() - time_before_frame);
		float step = Pi::timeAccel * Pi::frameTime;
		
		time_before_frame = SDL_GetTicks();
		// game state update crud
		if (step) {
			Space::TimeStep(step);
			gameTime += step;
		}
		current_view->Update();

		if (SDL_GetTicks() - last_stats > 1000) {
			snprintf(fps_readout, sizeof(fps_readout), "%d fps", frame_stat);
			frame_stat = 0;
			last_stats += 1000;
		}
	}
}

StarSystem *Pi::GetSelectedSystem()
{
	int sector_x, sector_y, system_idx;
	Pi::sector_view->GetSelectedSystem(&sector_x, &sector_y, &system_idx);
	if (system_idx == -1) {
		selected_system = 0;
		return NULL;
	}
	if (selected_system) {
		if (!selected_system->IsSystem(sector_x, sector_y, system_idx)) {
			delete selected_system;
			selected_system = 0;
		}
	}
	if (!selected_system) {
		selected_system = new StarSystem(sector_x, sector_y, system_idx);
	}
	return selected_system;
}

void Pi::HyperspaceTo(StarSystem *dest)
{
	Space::Clear();
	Space::BuildSystem(dest);
	float ang = rng(M_PI);
	Pi::player->SetPosition(vector3d(sin(ang)*8*AU,cos(ang)*8*AU,0));
	dest->GetPos(&Pi::playerLoc);
}

IniConfig::IniConfig(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Could not open '%s'.\n", filename);
		Pi::Quit();
	}
	char buf[1024];
	while (fgets(buf, sizeof(buf), f)) {
		if (buf[0] == '#') continue;
		char *sep = strchr(buf, '=');
		char *kend = sep;
		if (!sep) continue;
		*sep = 0;
		// strip whitespace
		while (isspace(*(--kend))) *kend = 0;
		while (isspace(*(++sep))) *sep = 0;
		// snip \r, \n
		char *vend = sep;
		while (*(++vend)) if ((*vend == '\r') || (*vend == '\n')) { *vend = 0; break; }
		std::string key = std::string(buf);
		std::string val = std::string(sep);
		(*this)[key] = val;
	}
	fclose(f);
}

int main(int argc, char**)
{
	printf("Pioneer ultra high tech tech demo dude!\n");

	IniConfig cfg("config.ini");

	Pi::Init(cfg);
	Pi::MainLoop();
	return 0;
}
