#include "FaceVideoLink.h"
#include "StarSystem.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "StarSystem.h"

FaceVideoLink::FaceVideoLink(float w, float h) : VideoLink(w, h) {
	const SBody *sbody = Pi::player->GetDockedWith()->GetSBody();
	MTRand rand(sbody->seed); //thanks for pointing this out Tom.
	int eyes_seed  = sbody->seed ;
	//if (StationBBView()) {
	//	eyes_seed = 1;
	//}

	if (eyes_seed < 0) {
		eyes_seed  = eyes_seed * -1.0f ;
	}

	//int face_seed  = eyes_seed %20;
	int face_seed  = (eyes_seed>>2)%21;
	//int mouth_seed = eyes_seed %80 ;
	int mouth_seed = (eyes_seed>>4)%21 ;
	//int nose_seed  = eyes_seed ;
	int nose_seed  = (eyes_seed>>6)%21 ;
	//int hair_seed  = eyes_seed %100 ;
	int hair_seed  = (eyes_seed>>8)%21 ;
	//int extr1_seed = eyes_seed %240 ;
	int extr1_seed = (eyes_seed>>10)%3 ;
	//int extr2_seed = eyes_seed %1500 ;
	int extr2_seed = (eyes_seed>>12)%14 ;
	//int sex_seed   = rand.Double(0,1);
	int sex_seed   = (eyes_seed>>14)%2;
	//int race_seed  = eyes_seed %3 ;
	int race_seed  = (eyes_seed>>16)%2 ;//should be %3
	//int cloth_seed = eyes_seed %40 ;
	int cloth_seed = (eyes_seed>>18)%21 ;
	//eyes_seed      = rand.Int32(0,eyes_seed);
	//eyes_seed      = eyes_seed %20 ;
	eyes_seed	   = (eyes_seed>>20)%21 ;
	//mouth_seed     = rand.Int32(0,mouth_seed);
	//mouth_seed     = mouth_seed * 0.25f ;
	//nose_seed      = rand.Int32(0,nose_seed);
	//nose_seed      = nose_seed * rand.Int32(0,1);
	//nose_seed      = nose_seed %20 ;
	//hair_seed      = hair_seed * 0.2f ;
	//extr1_seed     = extr1_seed / 12.0f ;
	//extr2_seed     = extr2_seed / 75.0f ;
	//cloth_seed     = rand.Int32(0,cloth_seed);
	//cloth_seed     = cloth_seed %20 ;
	//race_seed      = 0 ;  //temp
	//extr1_seed     = extr1_seed %3 ;  //temp
	//extr2_seed     = rand.Int32(0,12) ; // temp
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip("Video link established");
	std::string str = stringf(64, PIONEER_DATA_DIR "/facegen/race_%d/head/head_%d_%d.png", race_seed, sex_seed, face_seed);
	printf("%s\n", str.c_str());
	m_face = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/race_%d/eyes/eyes_%d_%d.png", race_seed, sex_seed, eyes_seed);
	printf("%s\n", str.c_str());
	m_eyes = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/race_%d/nose/nose_%d_%d.png", race_seed, sex_seed, nose_seed);
	printf("%s\n", str.c_str());
	m_nose = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/race_%d/mouth/mouth_%d_%d.png", race_seed, sex_seed, mouth_seed);
	printf("%s\n", str.c_str());
	m_mouth = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/race_%d/hair/hair_%d_%d.png", race_seed, sex_seed, hair_seed);
	printf("%s\n", str.c_str());
	m_hair = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/clothes/cloth_%d.png", cloth_seed);
	printf("%s\n", str.c_str());
	m_cloth = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/accessories/acc_%d.png", extr1_seed);
	printf("%s\n", str.c_str());
	m_extr1 = new Gui::Image(("" + str).c_str());
	str = stringf(64, PIONEER_DATA_DIR "/facegen/backgrounds/background_%d.png", extr2_seed);
	printf("%s\n", str.c_str());
	m_extr2 = new Gui::Image(("" + str).c_str());
}

FaceVideoLink::~FaceVideoLink() {
	delete m_message;
	delete m_face;
	delete m_eyes;
	delete m_nose;
	delete m_mouth;
	delete m_hair;
	delete m_cloth;
	delete m_extr1;
	delete m_extr2;
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
	GetSize(size);
	m_extr2->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_extr2->Draw();	
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);
	m_face->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_face->Draw();	
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);
	m_cloth->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_cloth->Draw();
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
	m_extr1->GetSize(size);
	glTranslatef((295 - size[0])*0.5f, 285 - size[1], 0);
	m_extr1->Draw();
	glTranslatef((size[0] - 295)*0.5f, size[1] - 285, 0);
}


