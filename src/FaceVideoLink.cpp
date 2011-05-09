#include "FaceVideoLink.h"

#define FACE_WIDTH  295
#define FACE_HEIGHT 285

// XXX these shouldn't really be hardcoded. it'd be much nicer to poke through
// the facegen/ dir and figure out what we've got available. that or some
// config file
#define MAX_HEAD  20
#define MAX_EYES  20
#define MAX_NOSE  20
#define MAX_MOUTH 20
#define MAX_HAIR  20

#define MAX_CLOTHES     20
#define MAX_ACCESSORIES 3

#define MAX_BACKGROUND 13

static void _blit_image(SDL_Surface *s, const char *filename, int yoff)
{
	SDL_Surface *is = IMG_Load(filename);
	if (!is) {
		fprintf(stderr, "FaceVideoLink: couldn't load '%s'\n", filename);
		return;
	}

	SDL_Rect destrec = { (FACE_WIDTH-is->w-1)/2, yoff, 0, 0 };
	SDL_BlitSurface(is, NULL, s, &destrec);
	SDL_FreeSurface(is);
}

FaceVideoLink::FaceVideoLink(float w, float h, int flags, unsigned long seed) : VideoLink(w, h) {
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip("Video link established");

	if (seed == (unsigned long)-1) seed = time(NULL);
	MTRand rand(seed);

	//int race = rand.Int32(0,1);    // XXX should be 3?
	// XXX forcing race to 0 for the moment. it stays this way until race 1
	// has enough components for both genders
	int race = 0;

	int gender;
	switch (flags & GENDER_MASK) {
		case GENDER_MALE:
			gender = 0;
			break;
		case GENDER_FEMALE:
			gender = 1;
			break;
		case GENDER_RAND:
		default:
			gender = rand.Int32(0,1);
			break;
	}

	int head  = rand.Int32(0,MAX_HEAD);
	int eyes  = rand.Int32(0,MAX_EYES);
	int nose  = rand.Int32(0,MAX_NOSE);
	int mouth = rand.Int32(0,MAX_MOUTH);
	int hair  = rand.Int32(0,MAX_HAIR);

	int clothes = rand.Int32(0,MAX_CLOTHES);
	int accessories = rand.Int32(0,MAX_ACCESSORIES);

	int background = rand.Int32(0,MAX_BACKGROUND);

	char filename[1024];

	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, FACE_WIDTH, FACE_HEIGHT, 32, 0xff, 0xff00, 0xff0000, 0xff000000);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/backgrounds/background_%d.png", background);
	//printf("%s\n", filename);
	_blit_image(s, filename, 0);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/head/head_%d_%d.png", race, gender, head);
	//printf("%s\n", filename);
	_blit_image(s, filename, 0);

	if (!(flags & ARMOUR)) {
		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/clothes/cloth_%d_%d.png", race, gender, clothes);
		//printf("%s\n", filename);
		_blit_image(s, filename, 135);
	}

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/eyes/eyes_%d_%d.png", race, gender, eyes);
	//printf("%s\n", filename);
	_blit_image(s, filename, 42);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/nose/nose_%d_%d.png", race, gender, nose);
	//printf("%s\n", filename);
	_blit_image(s, filename, 89);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/mouth/mouth_%d_%d.png", race, gender, mouth);
	//printf("%s\n", filename);
	_blit_image(s, filename, 155);

	if (!(flags & ARMOUR)) {
		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/hair/hair_%d_%d.png", race, gender, hair);
		//printf("%s\n", filename);
		_blit_image(s, filename, 0);

		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/accessories/acc_%d.png", accessories);
		//printf("%s\n", filename);
		_blit_image(s, filename, -10);
	}
	else
		_blit_image(s, PIONEER_DATA_DIR "/facegen/armour.png", 0);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FACE_WIDTH, FACE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);

	SDL_FreeSurface(s);
}

FaceVideoLink::~FaceVideoLink() {
	delete m_message;

	glDeleteTextures(1, &m_tex);
}

void FaceVideoLink::Draw() {
	float size[2]; GetSize(size);
	if (SDL_GetTicks() - m_created < 1500) {
		m_message->SetText("Connecting...");
		glBegin(GL_QUADS);
			glColor3f(0,0,0);
			glVertex2f(0,0);
			glVertex2f(0,size[1]);
			glVertex2f(size[0],size[1]);
			glVertex2f(size[0],0);
		glEnd();
		DrawMessage();
	} else {
		m_message->SetText("Video link established.");

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBegin(GL_QUADS);
			glColor3f(0,0,0);
			glTexCoord2f(0,0);
			glVertex2f(0,0);
			glTexCoord2f(0,1);
			glVertex2f(0,size[1]);
			glTexCoord2f(1,1);
			glVertex2f(size[0],size[1]);
			glTexCoord2f(1,0);
			glVertex2f(size[0],0);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		if (SDL_GetTicks() & 0x400)
			DrawMessage();
	}
}

void FaceVideoLink::DrawMessage() {
	float size[2];
	float msgSize[2];
	GetSize(size);
	m_message->GetSize(msgSize);
	glPushMatrix();
	if (SDL_GetTicks() - m_created < 1500) {
		glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
	} else {
		glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]-msgSize[1]*1.5f, 0);
	}
	
	m_message->Draw();
	glPopMatrix();
}
