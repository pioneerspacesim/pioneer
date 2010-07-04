#include "libs.h"
#include "glfreetype.h"
#include "Gui.h"
#include "collider/collider.h"
#include "LmrModel.h"
#include "Render.h"

static SDL_Surface *g_screen;
static int g_width, g_height;
static int g_mouseMotion[2];
static char g_keyState[SDLK_LAST];
static int g_mouseButton[5];
static LmrModel *g_model;
static float g_zbias;

static GLuint mytexture;

static void PollEvents()
{
	SDL_Event event;

	g_mouseMotion[0] = g_mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q) { SDL_Quit(); exit(0); }
				if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(g_screen);
				g_keyState[event.key.keysym.sym] = 1;
				break;
			case SDL_KEYUP:
				g_keyState[event.key.keysym.sym] = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				g_mouseButton[event.button.button] = 1;
	//			Pi::onMouseButtonDown.emit(event.button.button,
	//					event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				g_mouseButton[event.button.button] = 0;
	//			Pi::onMouseButtonUp.emit(event.button.button,
	//					event.button.x, event.button.y);
				break;
			case SDL_MOUSEMOTION:
				g_mouseMotion[0] += event.motion.xrel;
				g_mouseMotion[1] += event.motion.yrel;
				break;
			case SDL_QUIT:
				SDL_Quit();
				exit(0);
				break;
		}
	}
}

static int g_wheelMoveDir = -1;
static float g_wheelPos = 0;
static int g_renderType = 0;
static float lightCol[4] = { 1,1,1,0 };
static float lightDir[4] = { 0,1,0,0 };
static float g_frameTime;
static LmrObjParams params = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "IR-L33T", "ME TOO" },
	{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { .2f, .2f, .5f, 1 }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
	{ { 0.5f, 0.5f, 0.5f, 1 }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.8f, 0.8f, 1 }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
};
static LmrCollMesh *cmesh;
static Geom *geom;
static CollisionSpace *space;

class Viewer: public Gui::Fixed {
public:
	Gui::Adjustment *m_linthrust[3];
	Gui::Adjustment *m_angthrust[3];
	Gui::Adjustment *m_anim[LMR_ARG_MAX];
	Gui::TextEntry *m_animEntry[LMR_ARG_MAX];

	float GetAnimValue(int i) {
		std::string val = m_animEntry[i]->GetText();
		return (float)atof(val.c_str());
	}

	Viewer(): Gui::Fixed((float)g_width, (float)g_height) {
		Gui::Screen::AddBaseWidget(this, 0, 0);
		SetTransparency(true);
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_c, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnClickChangeView));
			Add(b, 10, 10);
			Add(new Gui::Label("[c] Change view (normal, collision mesh, raytraced collision mesh)"), 30, 10);
		} 
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_r, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnResetAdjustments));
			Add(b, 10, 30);
			Add(new Gui::Label("[r] Reset thruster and anim sliders"), 30, 30);
		} 
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_m, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnClickRebuildCollMesh));
			Add(b, 10, 50);
			Add(new Gui::Label("[m] Rebuild collision mesh"), 30, 50);
		} 
#if 0
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_g, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnToggleGearState));
			Add(b, 10, 30);
			Add(new Gui::Label("[g] Toggle gear state"), 30, 30);
		}
#endif /* 0 */	
		{
			Add(new Gui::Label("Linear thrust"), 0, 480);
			for (int i=0; i<3; i++) {
				m_linthrust[i] = new Gui::Adjustment();
				m_linthrust[i]->SetValue(0.5);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_linthrust[i]);
				Add(v, (float)(i*25), 500);
			}
			
			Add(new Gui::Label("Angular thrust"), 100, 480);
			for (int i=0; i<3; i++) {
				m_angthrust[i] = new Gui::Adjustment();
				m_angthrust[i]->SetValue(0.5);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_angthrust[i]);
				Add(v, (float)(100 + i*25), 500);
			}
			
			Add(new Gui::Label("Animations (0 gear, 1-4 are time - ignore them comrade)"), 200, 460);
			for (int i=0; i<LMR_ARG_MAX; i++) {
				Gui::Fixed *box = new Gui::Fixed(32, 119);
				Add(box, (float)(200 + i*25), 480);

				m_anim[i] = new Gui::Adjustment();
				m_anim[i]->SetValue(0);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_anim[i]);
				box->Add(v, 0, 42.0f);
				char buf[32];
				snprintf(buf, sizeof(buf), "%d", i);
				box->Add(new Gui::Label(buf), 0, 0);

				m_animEntry[i] = new Gui::TextEntry();
				box->Add(m_animEntry[i], 0, 16.0f);
				m_anim[i]->onValueChanged.connect(sigc::bind(sigc::mem_fun(this, &Viewer::OnAnimChange), m_anim[i], m_animEntry[i]));
				OnAnimChange(m_anim[i], m_animEntry[i]);
			}
		}

		ShowAll();
		Show();
	}

	void OnAnimChange(Gui::Adjustment *a, Gui::TextEntry *e) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%.2f", a->GetValue());
		e->SetText(buf);
	}

	void OnResetAdjustments() {
		for (int i=0; i<LMR_ARG_MAX; i++) m_anim[i]->SetValue(0);
		for (int i=0; i<3; i++) {
			m_linthrust[i]->SetValue(0.5);
			m_angthrust[i]->SetValue(0.5);
		}
	}
	void OnClickRebuildCollMesh() {
		space->RemoveGeom(geom);
		delete geom;
		delete cmesh;

		cmesh = new LmrCollMesh(g_model, &params);
		geom = new Geom(cmesh->geomTree);
		space->AddGeom(geom);
	}

	void OnToggleGearState() {
		if (g_wheelMoveDir == -1) g_wheelMoveDir = +1;
		else g_wheelMoveDir = -1;
	}

	void OnClickChangeView() {
		g_renderType++;
		if (g_renderType > 2) g_renderType = 0;
	}

	void MainLoop();
	void SetSbreParams();
};

void Viewer::SetSbreParams()
{
	float gameTime = SDL_GetTicks() * 0.001f;

	for (int i=0; i<LMR_ARG_MAX; i++) {
		params.argFloats[i] = GetAnimValue(i);
	}

	params.argFloats[1] = gameTime;
	params.argFloats[2] = gameTime / 60;
	params.argFloats[3] = gameTime / 3600.0f;
	params.argFloats[4] = gameTime / (24*3600.0f);
	
	params.linthrust[0] = 2.0f * (m_linthrust[0]->GetValue() - 0.5f);
	params.linthrust[1] = 2.0f * (m_linthrust[1]->GetValue() - 0.5f);
	params.linthrust[2] = 2.0f * (m_linthrust[2]->GetValue() - 0.5f);
	params.angthrust[0] = 2.0f * (m_angthrust[0]->GetValue() - 0.5f);
	params.angthrust[1] = 2.0f * (m_angthrust[1]->GetValue() - 0.5f);
	params.angthrust[2] = 2.0f * (m_angthrust[2]->GetValue() - 0.5f);
}


static void render_coll_mesh(const LmrCollMesh *m)
{
	glDisable(GL_LIGHTING);
	glColor3f(1,0,1);
	glDepthRange(0.0+g_zbias,1.0);
	glBegin(GL_TRIANGLES);
	for (int i=0; i<m->ni; i+=3) {
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
	}
	glEnd();
	glColor3f(1,1,1);
	glDepthRange(0,1.0f-g_zbias);
	for (int i=0; i<m->ni; i+=3) {
		glBegin(GL_LINE_LOOP);
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
		glEnd();
	}
	glDepthRange(0,1);
	glEnable(GL_LIGHTING);
}

#define TEXSIZE 512
float wank[TEXSIZE][TEXSIZE];
float aspectRatio = 1.0;
double camera_zoom = 1.0;
float distance = 100;
extern int stat_rayTriIntersections;
static void raytraceCollMesh(vector3d camPos, vector3d camera_up, vector3d camera_forward, CollisionSpace *space)
{
	memset(wank, 0, sizeof(float)*TEXSIZE*TEXSIZE);

	vector3d toPoint, xMov, yMov;

	vector3d topLeft, topRight, botRight, cross;
	topLeft = topRight = botRight = camera_forward * camera_zoom;
	cross = vector3d::Cross (camera_forward, camera_up) * aspectRatio;
	topLeft = topLeft + camera_up - cross;
	topRight = topRight + camera_up + cross;
	botRight = botRight - camera_up + cross;

	xMov = topRight - topLeft;
	yMov = botRight - topRight;
	float xstep = 1.0f / TEXSIZE;
	float ystep = 1.0f / TEXSIZE;
	float xpos, ypos;
	ypos = 0.0f;
	GeomTree::stats_rayTriIntersections = 0;

	Uint32 t = SDL_GetTicks();
	for (int y=0; y<TEXSIZE; y++, ypos += ystep) {
		xpos = 0.0f;
		for (int x=0; x<TEXSIZE; x++, xpos += xstep) {
			toPoint = (topLeft + (xMov * xpos) + (yMov * ypos)).Normalized();
			
			CollisionContact c;
			space->TraceRay(camPos, toPoint, 1000000.0f, &c);

			if (c.triIdx != -1) {
				wank[x][y] = distance/(10*c.dist);
			} else {
				wank[x][y] = 0;
			}
		}
	}
	printf("%.3f million rays/sec, %.2f tri isect tests per ray\n", (TEXSIZE*TEXSIZE)/(1000.0*(SDL_GetTicks()-t)),
				GeomTree::stats_rayTriIntersections/(float)(TEXSIZE*TEXSIZE));
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mytexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_FLOAT, wank);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//glActiveTexture(GL_TEXTURE0);
	glDisable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2i(0,1);
		glVertex3f(1,1,0);

		glTexCoord2i(0,0);
		glVertex3f(0,1,0);
		
		glTexCoord2i(1,0);
		glVertex3f(0,0,0);
		
		glTexCoord2i(1,1);
		glVertex3f(1,0,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

static void onCollision(CollisionContact *c)
{
	printf("depth %f\n", c->depth);
//	printf("%d: %d\n", SDL_GetTicks(), c->triIdx);
}

void Viewer::MainLoop()
{
	matrix4x4d rot = matrix4x4d::Identity();
	Uint32 lastTurd = SDL_GetTicks();

	Uint32 t = SDL_GetTicks();
	int numFrames = 0;
	Uint32 lastFpsReadout = SDL_GetTicks();
	cmesh = new LmrCollMesh(g_model, &params);
	distance = cmesh->GetBoundingRadius();
	

	printf("Geom tree build in %dms\n", SDL_GetTicks() - t);
	geom = new Geom(cmesh->geomTree);
	space = new CollisionSpace();
	space->AddGeom(geom);
//	geom2->MoveTo(rot, vector3d(80,0,0));

	Render::State rstate;
	rstate.SetZnearZfar(1.0f, 10000.0f);
	Render::SetCurrentState(&rstate);

	for (;;) {
		PollEvents();

		if (g_keyState[SDLK_UP]) rot = matrix4x4d::RotateXMatrix(g_frameTime) * rot;
		if (g_keyState[SDLK_DOWN]) rot = matrix4x4d::RotateXMatrix(-g_frameTime) * rot;
		if (g_keyState[SDLK_LEFT]) rot = matrix4x4d::RotateYMatrix(g_frameTime) * rot;
		if (g_keyState[SDLK_RIGHT]) rot = matrix4x4d::RotateYMatrix(-g_frameTime) * rot;
		if (g_keyState[SDLK_EQUALS]) distance *= powf(0.5f,g_frameTime);
		if (g_keyState[SDLK_MINUS]) distance *= powf(2.0f,g_frameTime);
		if (g_mouseButton[3]) {
			float rx = -0.01f*g_mouseMotion[1];
			float ry = -0.01f*g_mouseMotion[0];
			rot = matrix4x4d::RotateXMatrix(rx) * rot;
			rot = matrix4x4d::RotateYMatrix(ry) * rot;
		}
		geom->MoveTo(rot, vector3d(0.0,0.0,0.0));

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float fracH = g_height / (float)g_width;
		glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		SetSbreParams();
	
		if (g_renderType == 0) {
			glPushAttrib(GL_ALL_ATTRIB_BITS);
		
			matrix4x4f _m;
			for (int i=0; i<16; i++) _m[i] = rot[i];
			_m[14] = -distance;
			g_model->Render(_m, &params);
			glPopAttrib();
		} else if (g_renderType == 1) {
			glPushMatrix();
			glTranslatef(0, 0, -distance);
			glMultMatrixd(&rot[0]);
			render_coll_mesh(cmesh);
			glPopMatrix();
		} else {
			vector3d camPos = vector3d(0.0,0.0,(double)distance);
			vector3d forward = vector3d(0.0,0.0,-1.0);
			vector3d up = vector3d(0.0,1.0,0.0);
			raytraceCollMesh(camPos, up, forward, space);
		}
		
		Gui::Draw();
		
		glError();
		SDL_GL_SwapBuffers();
		numFrames++;
		g_frameTime = (SDL_GetTicks() - lastTurd) * 0.001f;
		lastTurd = SDL_GetTicks();
	
		if (SDL_GetTicks() - lastFpsReadout > 1000) {
			int numTris = LmrModelGetStatsTris();
			LmrModelClearStatsTris();
			printf("%d fps, %.3f Million tris/sec\n", numFrames, numTris/1000000.0f);
			numFrames = 0;
			lastFpsReadout = SDL_GetTicks();
		}

		space->Collide(onCollision);
	}
	delete cmesh;
}

extern void LmrModelCompilerInit();

int main(int argc, char **argv)
{
	if ((argc<=1) || (0==strcmp(argv[1],"--help"))) {
		printf("Usage:\nluamodelviewer <model name> <width> <height>\n");
		exit(0);
	}
	if (argc == 4) {
		g_width = atoi(argv[2]);
		g_height = atoi(argv[3]);
	} else {
		g_width = 800;
		g_height = 600;
	}

	const SDL_VideoInfo *info = NULL;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
		exit(-1);
	}

	info = SDL_GetVideoInfo();

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	g_zbias = 2.0/(1<<16);

	Uint32 flags = SDL_OPENGL;

	if ((g_screen = SDL_SetVideoMode(g_width, g_height, info->vfmt->BitsPerPixel, flags)) == 0) {
		// fall back on 16-bit depth buffer...
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer.\n", SDL_GetError());
		if ((g_screen = SDL_SetVideoMode(g_width, g_height, info->vfmt->BitsPerPixel, flags)) == 0) {
			fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
		}
	}
	glewInit();

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	glGenTextures(1, &mytexture);
	glBindTexture(GL_TEXTURE_2D, mytexture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glClearColor(0,0,0,0);
	glViewport(0, 0, g_width, g_height);
	GLFTInit();
	Render::Init();
	LmrModelCompilerInit();
	Gui::Init(g_width, g_height, g_width, g_height);
	if (argc > 1) {
		g_model = LmrLookupModelByName(argv[1]);
	}

	Viewer v;
	v.MainLoop();
	return 0;
}
