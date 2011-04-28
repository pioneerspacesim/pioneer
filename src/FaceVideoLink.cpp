#include "FaceVideoLink.h"

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

#define MAX_BACKGROUND 12

FaceVideoLink::FaceVideoLink(float w, float h, unsigned long seed) : VideoLink(w, h) {
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip("Video link established");

	MTRand rand(seed);

	int race = rand.Int32(0,1);    // XXX should be 3?
	int gender = rand.Int32(0,1);

	int head  = rand.Int32(0,MAX_HEAD);
	int eyes  = rand.Int32(0,MAX_EYES);
	int nose  = rand.Int32(0,MAX_NOSE);
	int mouth = rand.Int32(0,MAX_MOUTH);
	int hair  = rand.Int32(0,MAX_HAIR);

	int clothes = rand.Int32(0,MAX_CLOTHES);
	int accessories = rand.Int32(0,MAX_ACCESSORIES);

	int background = rand.Int32(0,MAX_BACKGROUND);

	char filename[1024];

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/head/head_%d_%d.png", race, gender, head);
	printf("%s\n", filename);
	m_head = new Gui::Image(filename);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/eyes/eyes_%d_%d.png", race, gender, eyes);
	printf("%s\n", filename);
	m_eyes = new Gui::Image(filename);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/nose/nose_%d_%d.png", race, gender, nose);
	printf("%s\n", filename);
	m_nose = new Gui::Image(filename);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/mouth/mouth_%d_%d.png", race, gender, mouth);
	printf("%s\n", filename);
	m_mouth = new Gui::Image(filename);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/hair/hair_%d_%d.png", race, gender, hair);
	printf("%s\n", filename);
	m_hair = new Gui::Image(filename);


	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/clothes/cloth_%d.png", clothes);
	printf("%s\n", filename);
	m_clothes = new Gui::Image(filename);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/accessories/acc_%d.png", accessories);
	printf("%s\n", filename);
	m_accessories = new Gui::Image(filename);


	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/backgrounds/background_%d.png", background);
	printf("%s\n", filename);
	m_background = new Gui::Image(filename);
}

FaceVideoLink::~FaceVideoLink() {
	delete m_message;

	delete m_head;
	delete m_eyes;
	delete m_nose;
	delete m_mouth;
	delete m_hair;
	delete m_clothes;
	delete m_accessories;
	delete m_background;
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

		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
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
		glDisable (GL_TEXTURE_2D);
		DrawFace();
		if (SDL_GetTicks() & 0x400) {
			DrawMessage();
		}
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

void FaceVideoLink::DrawFace() {
	float size[2];

	m_background->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_background->Draw();	
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);

	m_head->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_head->Draw();	
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);

	m_clothes->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_clothes->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);

	m_eyes->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 189 - size[1], 0);
	m_eyes->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] -189, 0);

	m_nose->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 221.5 - size[1], 0);
	m_nose->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] -221.5, 0);

	m_mouth->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 225 - size[1], 0);
	m_mouth->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] -225, 0);

	m_hair->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_hair->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);

	m_accessories->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_accessories->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);
}


