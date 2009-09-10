#include "libs.h"
#include "Pi.h"
#include "Gui.h"
#include "glfreetype.h"
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
#include "ObjectViewerView.h"
#include "StarSystem.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "CargoBody.h"
#include "InfoView.h"
#include "Serializer.h"
#include "NameGenerator.h"
#include "GeoSphere.h"
#include "Shader.h"

float Pi::timeAccel = 1.0f;
int Pi::scrWidth;
int Pi::scrHeight;
float Pi::scrAspect;
SDL_Surface *Pi::scrSurface;
sigc::signal<void, SDL_keysym*> Pi::onKeyPress;
sigc::signal<void, SDL_keysym*> Pi::onKeyRelease;
sigc::signal<void, int, int, int> Pi::onMouseButtonUp;
sigc::signal<void, int, int, int> Pi::onMouseButtonDown;
sigc::signal<void> Pi::onPlayerChangeHyperspaceTarget;
sigc::signal<void> Pi::onPlayerHyperspaceToNewSystem;
sigc::signal<void> Pi::onPlayerMissionListChanged;
sigc::signal<void, const SpaceStation*> Pi::onDockingClearanceExpired;
char Pi::keyState[SDLK_LAST];
char Pi::mouseButton[5];
int Pi::mouseMotion[2];
enum Pi::MapView Pi::mapView;
Player *Pi::player;
View *Pi::currentView;
WorldView *Pi::worldView;
ObjectViewerView *Pi::objectViewerView;
SpaceStationView *Pi::spaceStationView;
InfoView *Pi::infoView;
SectorView *Pi::sectorView;
SystemView *Pi::systemView;
SystemInfoView *Pi::systemInfoView;
ShipCpanel *Pi::cpan;
StarSystem *Pi::selectedSystem;
StarSystem *Pi::currentSystem;
MTRand Pi::rng;
double Pi::gameTime;
float Pi::frameTime;
GLUquadric *Pi::gluQuadric;
bool Pi::showDebugInfo;
int Pi::statSceneTris;

void Pi::Init(IniConfig &config)
{
	int width = config.Int("ScrWidth");
	int height = config.Int("ScrHeight");
	const SDL_VideoInfo *info = NULL;
	Uint32 sdlInitFlags = SDL_INIT_VIDEO;
#if defined _WIN32 && defined _DEBUG
	sdlInitFlags |= SDL_INIT_NOPARACHUTE;
#endif
	if (SDL_Init(sdlInitFlags) < 0) {
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
	glewInit();
	SDL_WM_SetCaption("Pioneer","Pioneer");
	Pi::scrWidth = width;
	Pi::scrHeight = height;
	Pi::scrAspect = width / (float)height;

	Pi::rng.seed(time(NULL));

	sbreCompilerLoadModels();
	NameGenerator::Init();
	InitOpenGL();
	if (config.Int("UseVertexShaders")) Shader::Init();

	GLFTInit();
	GeoSphere::Init();
	Space::Init();
	
	Gui::Init(scrWidth, scrHeight, 800, 600);
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
	
	fprintf(stderr, "GL_ARB_vertex_buffer_object: %s\n", GLEW_ARB_vertex_buffer_object ? "Yes" : "No");
}

void Pi::Quit()
{
	SDL_Quit();
	exit(0);
}

void Pi::SetTimeAccel(float s)
{
	// don't want player to spin like mad when hitting time accel
	if (s > 10) {
		player->SetAngVelocity(vector3d(0,0,0));
		player->SetTorque(vector3d(0,0,0));
		player->SetAngThrusterState(0, 0.0f);
		player->SetAngThrusterState(1, 0.0f);
		player->SetAngThrusterState(2, 0.0f);
	}
	timeAccel = s;
}

void Pi::SetMapView(enum MapView v)
{
	mapView = v;
	if (v == MAP_SECTOR)
		SetView(sectorView);
	else
		SetView(systemView);
}

void Pi::SetView(View *v)
{
	if (currentView) currentView->HideAll();
	currentView = v;
	currentView->OnSwitchTo();
	currentView->ShowAll();
}

void Screendump(char *destFile)
{
	/* XXX TODO XXX not endian-safe */
	const int W = Pi::GetScrWidth();
	const int H = Pi::GetScrHeight();
	std::vector<char> pixel_data(3*W*H);
	short TGAhead[] = {0, 2, 0, 0, 0, 0, W, H, 24};
	FILE *out = fopen(destFile, "w");
	if (!out) goto error;
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, W, H, GL_BGR, GL_UNSIGNED_BYTE, &pixel_data[0]);
	if (fwrite(&TGAhead, sizeof(TGAhead), 1, out) != 1) goto error;
	if (fwrite(&pixel_data[0], 3*W*H, 1, out) != 1) goto error;
	fclose(out);
	return;
error:
	printf("Failed to write screendump.\n");
}

void Pi::HandleEvents()
{
	SDL_Event event;

	Pi::mouseMotion[0] = Pi::mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		switch (event.type) {
			case SDL_KEYDOWN:
				// special keys. LCTRL+turd
				if ((KeyState(SDLK_LCTRL) || (KeyState(SDLK_RCTRL)))) {
					if (event.key.keysym.sym == SDLK_q) Pi::Quit();
					if (event.key.keysym.sym == SDLK_s) {
						Shader::ToggleState();
					}
					if (event.key.keysym.sym == SDLK_i) Pi::showDebugInfo = !Pi::showDebugInfo;
					if (event.key.keysym.sym == SDLK_PRINT) {
						char buf[256];
						const time_t t = time(0);
						struct tm *_tm = localtime(&t);
						strftime(buf, sizeof(buf), "screenshot-%Y%m%d-%H%M%S.tga", _tm);
						Screendump(buf);
						fprintf(stderr, "Screendump to %s\n", buf);
					}
#ifdef DEBUG
					if (event.key.keysym.sym == SDLK_m) {
						Pi::player->SetMoney(Pi::player->GetMoney() + 10000000);
					}
					if (event.key.keysym.sym == SDLK_F12) {
						matrix4x4d m; Pi::player->GetRotMatrix(m);
						vector3d dir = m*vector3d(0,0,-1);
						/* add test object */
						if (KeyState(SDLK_RSHIFT)) {
							CargoBody *cargo = new CargoBody(Equip::HYDROGEN);
							cargo->SetFrame(Pi::player->GetFrame());
							cargo->SetPosition(Pi::player->GetPosition()+100.0*dir);
							cargo->SetVelocity(Pi::player->GetVelocity());
							Space::AddBody(cargo);
						} else {
							Ship *ship = new Ship(ShipType::LADYBIRD);
							ship->AIInstruct(Ship::DO_KILL, Pi::player);
							ship->SetFrame(Pi::player->GetFrame());
							ship->SetPosition(Pi::player->GetPosition()+100.0*dir);
							ship->SetVelocity(Pi::player->GetVelocity());
							Space::AddBody(ship);
						}
					}
#endif /* DEBUG */
					if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(Pi::scrSurface);
					if (event.key.keysym.sym == SDLK_F10) Pi::SetView(Pi::objectViewerView);
					if (event.key.keysym.sym == SDLK_F9) Serializer::Write::Game("quicksave.sav");
				}
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

static void draw_intro(float _time)
{
	static float lightCol[4] = { 1,1,1,0 };
	static float lightDir[4] = { 0,1,0,0 };

	static ObjParams params = {
		{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f },
		{	// pColor[3]
		{ { .2f, .2f, .5f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
		{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.8f, 0.8f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
		{ "PIONEER" },
	};
	glRotatef(_time*10, 1, 0, 0);
	Pi::worldView->DrawBgStars();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDepthRange(Pi::GetScrWidth()*0.5, 0.0f, 1.0f);
	sbreSetDirLight (lightCol, lightDir);
	matrix4x4d rot = matrix4x4d::RotateYMatrix(_time) * matrix4x4d::RotateZMatrix(0.6*_time) *
			matrix4x4d::RotateXMatrix(_time*.7);
	vector3d p(0, 0, -80);
	sbreRenderModel(&p.x, &rot[0], 61, &params);
	glPopAttrib();
}

static void draw_tombstone(float _time)
{
	static float lightCol[4] = { 1,1,1,0 };
	static float lightDir[4] = { 0,1,0,0 };

	static ObjParams params = {
		{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },
		{	// pColor[3]
		{ { 1.0f, 1.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
		{ "RIP OLD BEAN" },
	};
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDepthRange(Pi::GetScrWidth()*0.5, 0.0f, 1.0f);
	sbreSetDirLight (lightCol, lightDir);
	matrix4x4d rot = matrix4x4d::RotateYMatrix(_time*2);
	vector3d p(0, 0, -MAX(150 - 30*_time, 30));
	sbreRenderModel(&p.x, &rot[0], 91, &params);
	glPopAttrib();
}

void Pi::TombStoneLoop()
{
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	cpan->HideAll();
	currentView->HideAll();
	do {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float fracH = 1.0 / Pi::GetScrAspect();
		glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Pi::HandleEvents();
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);

		draw_tombstone(_time);
		Gui::Draw();
		glFlush();
		SDL_GL_SwapBuffers();
		
		Pi::frameTime = 0.001*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	} while (!((_time > 2.0) && ((Pi::MouseButtonState(1)) || Pi::KeyState(SDLK_SPACE)) ));
}

void Pi::Start()
{
	// this is a bit brittle. skank may be forgotten and survive between
	// games
	Pi::timeAccel = 1.0f;
	Pi::gameTime = 0;
	Pi::currentView = 0;

	player = new Player(ShipType::SIRIUS_INTERDICTOR);
	player->m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS3);
	player->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::LASER_2MW_BEAM);
	player->m_equipment.Set(Equip::SLOT_LASER, 1, Equip::LASER_4MW_BEAM);
	player->m_equipment.Add(Equip::SLOT_CARGO, Equip::HYDROGEN);
	player->m_equipment.Add(Equip::SLOT_CARGO, Equip::HYDROGEN);
	player->m_equipment.Add(Equip::SLOT_CARGO, Equip::HYDROGEN);
	player->SetMoney(10000);
	Space::AddBody(player);
	
	cpan = new ShipCpanel();
	sectorView = new SectorView();
	systemView = new SystemView();
	systemInfoView = new SystemInfoView();
	worldView = new WorldView();
	objectViewerView = new ObjectViewerView();
	spaceStationView = new SpaceStationView();
	infoView = new InfoView();

	Gui::Fixed *splash = new Gui::Fixed(Gui::Screen::GetWidth(), Gui::Screen::GetHeight());
	Gui::Screen::AddBaseWidget(splash, 0, 0);
	splash->SetTransparency(true);

	const float w = Gui::Screen::GetWidth() / 2;
	const float h = Gui::Screen::GetHeight() / 2;
	const int OPTS = 4;
	Gui::ToggleButton *opts[OPTS];
	opts[0] = new Gui::ToggleButton(); opts[0]->SetShortcut(SDLK_1, KMOD_NONE);
	opts[1] = new Gui::ToggleButton(); opts[1]->SetShortcut(SDLK_2, KMOD_NONE);
	opts[2] = new Gui::ToggleButton(); opts[2]->SetShortcut(SDLK_3, KMOD_NONE);
	opts[3] = new Gui::ToggleButton(); opts[3]->SetShortcut(SDLK_4, KMOD_NONE);
	splash->Add(opts[0], w, h-64);
	splash->Add(new Gui::Label("New game starting on Earth"), w+32, h-64);
	splash->Add(opts[1], w, h-32);
	splash->Add(new Gui::Label("New game starting on debug point"), w+32, h-32);
	splash->Add(opts[2], w, h);
	splash->Add(new Gui::Label("Load quicksave"), w+32, h);
	splash->Add(opts[3], w, h+32);
	splash->Add(new Gui::Label("Quit"), w+32, h+32);

	splash->ShowAll();

	int choice = 0;
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	do {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float fracH = 1.0 / Pi::GetScrAspect();
		glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Pi::HandleEvents();
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);

		draw_intro(_time);
		Gui::Draw();
		glFlush();
		SDL_GL_SwapBuffers();
		
		Pi::frameTime = 0.001*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();

		// poll ui instead of using callbacks :-J
		for (int i=0; i<OPTS; i++) if (opts[i]->GetPressed()) choice = i+1;
	} while (!choice);
	splash->HideAll();

	if (choice == 1) {
		/* Earth start point */
		SBodyPath path(0,0,0);
		Space::DoHyperspaceTo(&path);
		//Frame *pframe = *(++(++(Space::rootFrame->m_children.begin())));
		//player->SetFrame(pframe);
		// XXX there isn't a sensible way to find stations for a planet.
		SpaceStation *station = 0;
		for (Space::bodiesIter_t i = Space::bodies.begin(); i!=Space::bodies.end(); i++) {
			if ((*i)->IsType(Object::SPACESTATION)) { station = (SpaceStation*)*i; break; }
		}
		assert(station);
		player->SetPosition(vector3d(0,0,0));
		player->SetFrame(station->GetFrame());
		player->SetDockedWith(station, 0);
		MainLoop();
	} else if (choice == 2) {
		/* debug start point */
		SBodyPath path(1,0,2);
		path.elem[0] = 3;
		Space::DoHyperspaceTo(&path);
		player->SetPosition(vector3d(0,0,2*EARTH_RADIUS));
	/*	Frame *stationFrame = new Frame(pframe, "Station frame...");
		stationFrame->SetRadius(5000);
		stationFrame->m_sbody = 0;
		stationFrame->SetPosition(vector3d(0,0,zpos));
		stationFrame->SetAngVelocity(vector3d(0,0,0.5));

		for (int i=0; i<4; i++) {
			Ship *body = new Ship(ShipType::LADYBIRD);
			char buf[64];
			snprintf(buf,sizeof(buf),"X%c-0%02d", 'A'+i, i);
			body->SetLabel(buf);
			body->SetFrame(stationFrame);
			body->SetPosition(vector3d(200*(i+1), 0, 2000));
			Space::AddBody(body);
		}
			
		SpaceStation *station = new SpaceStation(SpaceStation::JJHOOP);
		station->SetLabel("Poemi-chan's Folly");
		station->SetFrame(stationFrame);
		station->SetPosition(vector3d(0,0,0));
		Space::AddBody(station);

		SpaceStation *station2 = new SpaceStation(SpaceStation::GROUND_FLAVOURED);
		station2->SetLabel("Conor's End");
		station2->SetFrame(*pframe->m_children.begin()); // rotating frame of planet
		station2->OrientOnSurface(EARTH_RADIUS, M_PI/4, M_PI/4);
		Space::AddBody(station2);
	*/
	//	player->SetDockedWith(station2, 0);

		MainLoop();
	} else if (choice == 3) {
		// load quicksave
		Serializer::Read::Game("quicksave.sav");
		MainLoop();
	} else {
		Pi::Quit();
	}
	
	Gui::Screen::RemoveBaseWidget(splash);
	delete splash;
	delete infoView;
	delete spaceStationView;
	delete objectViewerView;
	delete worldView;
	delete systemInfoView;
	delete systemView;
	delete sectorView;
	delete cpan;
	Space::RemoveBody(Pi::player);
	delete player;
}

void Pi::MainLoop()
{
	cpan->ShowAll();

	SetView(worldView);

	Uint32 last_stats = SDL_GetTicks();
	int frame_stat = 0;
	int phys_stat = 0;
	char fps_readout[128];
	Uint32 time_before_frame = SDL_GetTicks();
	Uint32 last_phys_update = time_before_frame;
	double time_player_died = 0;
#ifdef MAKING_VIDEO
	Uint32 last_screendump = SDL_GetTicks();
	int dumpnum = 0;
#endif /* MAKING_VIDEO */

	memset(fps_readout, 0, sizeof(fps_readout));

	for (;;) {
		frame_stat++;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		currentView->Draw3D();
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
//#ifdef DEBUG
		if (Pi::showDebugInfo) {
			Gui::Screen::EnterOrtho();
			glColor3f(1,1,1);
			Gui::Screen::RenderString(fps_readout);
			Gui::Screen::LeaveOrtho();
		}
//#endif /* DEBUG */

		glFlush();
		SDL_GL_SwapBuffers();
		//if (glGetError()) printf ("GL: %s\n", gluErrorString (glGetError ()));
		
		Pi::frameTime = 0.001*(SDL_GetTicks() - time_before_frame);
		time_before_frame = SDL_GetTicks();
		
		// Fixed 62.5hz physics
		int num_steps = 0;
		while (time_before_frame - last_phys_update > 16) {
			last_phys_update += 16;
			const float step = Pi::GetTimeStep();
			if (step) Space::TimeStep(step);
			gameTime += step;
			phys_stat++;
			if (++num_steps > 3) break;
		}
		// fuckadoodledoo, did the player die?
		if (Pi::player->IsDead()) {
			if (time_player_died) {
				if (Pi::GetGameTime() - time_player_died > 8.0) {
					Pi::TombStoneLoop();
					break;
				}
			} else {
				Pi::SetTimeAccel(1.0f);
				Pi::cpan->HideAll();
				Pi::SetView(static_cast<View*>(Pi::worldView));
				Pi::player->Disable();
				time_player_died = Pi::GetGameTime();
			}
		}
		currentView->Update();

		if (SDL_GetTicks() - last_stats > 1000) {
			snprintf(fps_readout, sizeof(fps_readout), "%d fps, %d phys updates, %d triangles, %.3f M tris/sec", frame_stat, phys_stat, Pi::statSceneTris, Pi::statSceneTris*frame_stat*1e-6);
			frame_stat = 0;
			phys_stat = 0;
			last_stats += 1000;
		}
		Pi::statSceneTris = 0;

#ifdef MAKING_VIDEO
		if (SDL_GetTicks() - last_screendump > 50) {
			last_screendump = SDL_GetTicks();
			char buf[256];
			snprintf(buf, sizeof(buf), "screenshot%08d.tga", dumpnum++);
			Screendump(buf);
		}
#endif /* MAKING_VIDEO */
	}
}

StarSystem *Pi::GetSelectedSystem()
{
	int sector_x, sector_y, system_idx;
	Pi::sectorView->GetSelectedSystem(&sector_x, &sector_y, &system_idx);
	if (system_idx == -1) {
		selectedSystem = 0;
		return NULL;
	}
	if (selectedSystem) {
		if (!selectedSystem->IsSystem(sector_x, sector_y, system_idx)) {
			delete selectedSystem;
			selectedSystem = 0;
		}
	}
	if (!selectedSystem) {
		selectedSystem = new StarSystem(sector_x, sector_y, system_idx);
	}
	return selectedSystem;
}

void Pi::Serialize()
{
	using namespace Serializer::Write;
	StarSystem::Serialize(selectedSystem);
	wr_double(gameTime);
	StarSystem::Serialize(currentSystem);
	Space::Serialize();
	sectorView->Save();
	worldView->Save();
}

void Pi::Unserialize()
{
	using namespace Serializer::Read;
	selectedSystem = StarSystem::Unserialize();
	gameTime = rd_double();
	currentSystem = StarSystem::Unserialize();
	SetTimeAccel(0);
	Space::Clear();
	if (Pi::player) {
		Pi::player->MarkDead();
		Space::bodies.remove(Pi::player);
		delete Pi::player;
		Pi::player = 0;
	}
	Space::Unserialize();
	sectorView->Load();
	worldView->Load();
}

IniConfig::IniConfig(const char *filename)
{
	FILE *f = fopen_or_die(filename, "r");
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

