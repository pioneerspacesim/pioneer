#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "GenericChatForm.h"

#define TEXSIZE	128

class DeadVideoLink: public Gui::Widget {
public:
	void PutRandomCrapIntoTexture() {
		int *randcrap = (int*)alloca(TEXSIZE*TEXSIZE);
		for (unsigned int i=0; i<TEXSIZE*TEXSIZE/sizeof(int); i++) randcrap[i] = (Pi::rng.Int32() & 0xfcfcfcfc) >> 2;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, randcrap);
	}
	DeadVideoLink(float w, float h) {
		m_w = w; m_h = h;
		m_created = SDL_GetTicks();
		m_message = new Gui::ToolTip("Video link down");
		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &m_tex);
		glBindTexture (GL_TEXTURE_2D, m_tex);
		PutRandomCrapIntoTexture();
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glDisable (GL_TEXTURE_2D);
	}
	virtual ~DeadVideoLink() {
		glDeleteTextures(1, &m_tex);
		delete m_message;
	}
	virtual void Draw() {
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
			m_message->SetText("Video link down");

			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_tex);
			PutRandomCrapIntoTexture();
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
			if (SDL_GetTicks() & 0x400) {
				DrawMessage();
			}
		}
	}
	virtual void GetSizeRequested(float size[2]) {
		size[0] = m_w;
		size[1] = m_h;
	}
private:
	void DrawMessage() {
		float size[2];
		float msgSize[2];
		GetSize(size);
		m_message->GetSize(msgSize);
		glPushMatrix();
		glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
		m_message->Draw();
		glPopMatrix();
	}
	Uint32 m_created;
	GLuint m_tex;
	float m_w, m_h;
	Gui::ToolTip *m_message;
};

/*class VideoLink: public Gui::Widget {
public:
	void PutRandomCrapIntoTexture() {
		int *randcrap = (int*)alloca(TEXSIZE*TEXSIZE);
		for (unsigned int i=0; i<TEXSIZE*TEXSIZE/sizeof(int); i++) randcrap[i] = (Pi::rng.Int32() & 0xfcfcfcfc) >> 2;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, randcrap);
	}
	void RandomFace() {
		MTRand rand;
		const SBody *sbody = Pi::player->GetDockedWith()->GetSBody();
		int face_seed  = sbody->seed %20;
		int eyes_seed  = sbody->seed ;
		int mouth_seed = eyes_seed %80 ;
		int nose_seed  = eyes_seed ;
		int hair_seed  = eyes_seed %100 ;
		int extra_seed = eyes_seed %240 ;
		eyes_seed  = rand.Int32(0,eyes_seed);
		eyes_seed  = eyes_seed %20 ;
		mouth_seed = rand.Int32(0,mouth_seed);
		mouth_seed = mouth_seed * 0.25f ;
		nose_seed  = rand.Int32(0,nose_seed);
		nose_seed  = nose_seed * rand.Int32(0,1);
		nose_seed  = nose_seed %20 ;
		hair_seed  = hair_seed * 0.2f ;
		extra_seed = extra_seed / 12.0f ;
		(new Gui::Image((PIONEER_DATA_DIR "/icons/faces/" + (stringf(64, "face%d", face_seed)) + ".png").c_str()), 10, 40);
		(new Gui::Label(stringf(64, "Face:  %d", face_seed)), 10, 50);
		(new Gui::Label(stringf(64, "Eyes:  %d", eyes_seed)), 10, 60);
		(new Gui::Label(stringf(64, "Mouth: %d", mouth_seed)), 10, 70);
		(new Gui::Label(stringf(64, "Nose:  %d", nose_seed)), 10, 80);
		(new Gui::Label(stringf(64, "Hair:  %d", hair_seed)), 10, 90);
		(new Gui::Label(stringf(64, "Extra: %d", extra_seed)), 10, 100);
		
	}
	VideoLink(float w, float h) {

		m_w = w; m_h = h;
		m_created = SDL_GetTicks();
		m_message = new Gui::ToolTip("Video link down");
		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &m_tex);
		glBindTexture (GL_TEXTURE_2D, m_tex);
		PutRandomCrapIntoTexture();
		//RandomFace();
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glDisable (GL_TEXTURE_2D);
	}
	virtual ~VideoLink() {
		glDeleteTextures(1, &m_tex);
		delete m_message;
	}
	virtual void Draw() {
		float size[2]; GetSize(size);
		if (SDL_GetTicks() - m_created < 1500) {
			m_message->SetText("Connecting...");
			//RandomFace();
			glBegin(GL_QUADS);
				glColor3f(0,0,0);
				glVertex2f(0,0);
				glVertex2f(0,size[1]);
				glVertex2f(size[0],size[1]);
				glVertex2f(size[0],0);
			glEnd();
			DrawMessage();
		} else {
			m_message->SetText("Video link down");

			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_tex);
			PutRandomCrapIntoTexture();
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
			if (SDL_GetTicks() & 0x400) {
				DrawMessage();
			}
		}
	}
	virtual void GetSizeRequested(float size[2]) {
		size[0] = m_w;
		size[1] = m_h;
	}
private:
	void DrawMessage() {
		float size[2];
		float msgSize[2];
		GetSize(size);
		m_message->GetSize(msgSize);
		glPushMatrix();
		glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
		m_message->Draw();
		glPopMatrix();
	}
	Uint32 m_created;
	GLuint m_tex;
	float m_w, m_h;
	Gui::ToolTip *m_message;
};*/

void GenericChatForm::AddFaceWidget()
{
	//SpaceStation *name = Pi::player->GetDockedWith();

	//This crap is to do with randomly generating faces
	MTRand rand;
	const SBody *sbody = Pi::player->GetDockedWith()->GetSBody();
	int eyes_seed  = sbody->seed ;

	if (eyes_seed < 0) {
		eyes_seed  = eyes_seed * -1.0f ;
	}

	int face_seed  = eyes_seed %20;
	int mouth_seed = eyes_seed %80 ;
	int nose_seed  = eyes_seed ;
	int hair_seed  = eyes_seed %100 ;
	int extr1_seed = eyes_seed %240 ;
	int extr2_seed = eyes_seed %1500 ;
	int sex_seed   = eyes_seed %1 ;
	int race_seed  = eyes_seed %3 ;
	int cloth_seed = eyes_seed %40 ;
	eyes_seed      = rand.Int32(0,eyes_seed);
	eyes_seed      = eyes_seed %20 ;
	mouth_seed     = rand.Int32(0,mouth_seed);
	mouth_seed     = mouth_seed * 0.25f ;
	nose_seed      = rand.Int32(0,nose_seed);
	nose_seed      = nose_seed * rand.Int32(0,1);
	nose_seed      = nose_seed %20 ;
	hair_seed      = hair_seed * 0.2f ;
	extr1_seed     = extr1_seed / 12.0f ;
	extr2_seed     = extr2_seed / 75.0f ;
	cloth_seed     = rand.Int32(0,cloth_seed);
	cloth_seed     = cloth_seed %20 ;
	race_seed      = 0 ;

	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/"
		+ (stringf(64, "race_%d" , race_seed))
		+ (stringf(64, "/head/head_%d" , sex_seed))
		+ (stringf(64, "_%d" , face_seed)) + ".png").c_str()), 10, 40);
	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/"   
		+ (stringf(64, "race_%d" , race_seed))
		+ (stringf(64, "/eyes/eyes_%d" , sex_seed))
		+ (stringf(64, "_%d" , face_seed)) + ".png").c_str()), 89, 144);
	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/" 
		+ (stringf(64, "race_%d" , race_seed))
		+ (stringf(64, "/nose/nose_%d" , sex_seed))
		+ (stringf(64, "_%d" , face_seed)) + ".png").c_str()), 123, 192);
	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/"  
		+ (stringf(64, "race_%d" , race_seed))
		+ (stringf(64, "/mouth/mouth_%d" , sex_seed))
		+ (stringf(64, "_%d", face_seed)) + ".png").c_str()), 114, 224);
	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/"  
		+ (stringf(64, "race_%d" , race_seed))
		+ (stringf(64, "/hair/hair_%d" , sex_seed))
		+ (stringf(64, "_%d" , face_seed)) + ".png").c_str()), 10, 40);
	Add(new Gui::Image((PIONEER_DATA_DIR "/icons/facegen/clothes/"
		+ (stringf(64, "cloth_%d", face_seed)) + ".png").c_str()), 10, 185);

	
	Add(new Gui::Label(stringf(64, "Face:   %d", face_seed )), 10, 50);
	Add(new Gui::Label(stringf(64, "Eyes:   %d", eyes_seed )), 10, 60);
	Add(new Gui::Label(stringf(64, "Mouth:  %d", mouth_seed)), 10, 70);
	Add(new Gui::Label(stringf(64, "Nose:   %d", nose_seed )), 10, 80);
	Add(new Gui::Label(stringf(64, "Hair:   %d", hair_seed )), 10, 90);
	Add(new Gui::Label(stringf(64, "Clothes:%d", cloth_seed)), 10, 100);
	Add(new Gui::Label(stringf(64, "Extra1: %d", extr1_seed)), 10, 110);
	Add(new Gui::Label(stringf(64, "Extra2: %d", extr2_seed)), 10, 120);
	Add(new Gui::Label(stringf(64, "Sex:    %d", sex_seed  )), 10, 130);
	Add(new Gui::Label(stringf(64, "Race:   %d", race_seed )), 10, 140);

	onClose.emit(this);
}


void GenericChatForm::AddVideoWidget()
{
	Add(new DeadVideoLink(295,285), 5, 40);
	//Add(new VideoLink(295,285), 5, 40);
	 AddFaceWidget();
}


void GenericChatForm::Close()
{
	//GetParent()->RemoveChild(this);
	//GetParent()->ShowChildren();
	//GetParent()->SetTransparency(false);
	onClose.emit(this);
	//static_cast<GenericChatForm*>(GetParent())->UpdateBaseDisplay();
	//delete this;
}

void GenericChatForm::UpdateBaseDisplay()
{
	if (m_money) {
		char buf[64];
		m_money->SetText(format_money(Pi::player->GetMoney()));

		const shipstats_t *stats = Pi::player->CalcStats();
		snprintf(buf, sizeof(buf), "%dt", stats->used_capacity - stats->used_cargo);
		m_equipmentMass->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->used_cargo);
		m_cargoSpaceUsed->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->free_capacity);
		m_cargoSpaceFree->SetText(buf);
	}
}

void GenericChatForm::AddBaseDisplay()
{
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	const float ystart = 350.0f;

	Add(new Gui::Label("#007Cash"), 10, ystart);
	Add(new Gui::Label("#007Legal status"), 10, ystart + 2*YSEP);
	Add(new Gui::Label("#007Used"), 140, ystart+4*YSEP);
	Add(new Gui::Label("#007Free"), 220, ystart+4*YSEP);
	Add(new Gui::Label("#007Cargo space"), 10, ystart+5*YSEP);
	Add(new Gui::Label("#007Equipment"), 10, ystart+6*YSEP);

	m_money = new Gui::Label("");
	Add(m_money, 220, ystart);

	m_cargoSpaceUsed = new Gui::Label("");
	Add(m_cargoSpaceUsed, 140, ystart + 5*YSEP);
	
	m_cargoSpaceFree = new Gui::Label("");
	Add(m_cargoSpaceFree, 220, ystart + 5*YSEP);
	
	m_equipmentMass = new Gui::Label("");
	Add(m_equipmentMass, 140, ystart + 6*YSEP);
	
	m_legalstatus = new Gui::Label("Clean");
	Add(m_legalstatus, 220, ystart + 2*YSEP);

	UpdateBaseDisplay();
}

void GenericChatForm::OpenChildChatForm(GenericChatForm *form)
{
//	HideChildren();
//	//SetTransparency(true);
//	
//	Gui::Fixed *f = new Gui::Fixed(470, 400);
//	Add(f, 320, 40);
//	f->SetTransparency(false);
//	f->Add(form, 0, 0);
//	f->ShowAll();
	HideChildren();
	SetTransparency(true);
	Add(form, 0, 0);
	form->ShowAll();
	form->onClose.connect(sigc::mem_fun(this, &GenericChatForm::OnCloseChildChatForm));
}

void GenericChatForm::OnCloseChildChatForm(GenericChatForm *form)
{
	RemoveChild(form);
	SetTransparency(false);
	delete form;
	ShowAll();
}

GenericChatForm::GenericChatForm(): Gui::Fixed((float)Gui::Screen::GetWidth(), (float)(Gui::Screen::GetHeight()-64))
{
	ReInit();
}

void GenericChatForm::ReInit()
{
	DeleteAllChildren();
	m_legalstatus = 0;
	m_money = 0;
	m_cargoSpaceUsed = 0;
	m_cargoSpaceFree = 0;
	m_equipmentMass = 0;
	m_titleLabel = new Gui::Label("");
	Add(m_titleLabel, 10, 10);

	SetTransparency(false);
	m_chatRegion = new Gui::Fixed(470, 400);
	Add(m_chatRegion, 320, 40);

	m_msgregion = new Gui::VBox();
	m_optregion = new Gui::VBox();
	m_msgregion->SetSizeRequest(470, 150);
       	m_msgregion->SetSpacing(5.0f);
       	m_optregion->SetSpacing(5.0f);
	m_chatRegion->Add(m_msgregion, 0, 0);
	m_chatRegion->Add(m_optregion, 0, 160);
	m_msgregion->Show();
	m_optregion->Show();
	Clear();
}

void GenericChatForm::SetTitle(const char *title)
{
	m_titleLabel->SetText(title);
}

void GenericChatForm::Clear()
{
	m_msgregion->DeleteAllChildren();
	m_optregion->DeleteAllChildren();
	hasOpts = false;
}

void GenericChatForm::SetMessage(const char *msg)
{
	m_msgregion->PackEnd(new Gui::Label(msg));
	ShowAll();
}

void GenericChatForm::AddOption(sigc::slot<void,GenericChatForm*,int> slot, const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
//	b->onClick.connect(sigc::bind(sigc::mem_fun(m, &Mission::FormResponse), this, val));
	b->onClick.connect(sigc::bind(slot, this, val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}

