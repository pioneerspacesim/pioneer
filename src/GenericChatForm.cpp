#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "GenericChatForm.h"
#include "SpaceStation.h"
#include "VideoLink.h"

class FaceVideoLink: public VideoLink {
public:
	
	FaceVideoLink(float w, float h) : VideoLink(w, h) {
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
		m_w = w; m_h = h;
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
	virtual ~FaceVideoLink() {

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
		if (SDL_GetTicks() - m_created < 1500) {
			glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
		} else {
			glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]-msgSize[1]*1.5f, 0);
		}
		
		m_message->Draw();
		glPopMatrix();
	}
	void DrawFace() {
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
	Uint32 m_created;
	GLuint m_tex;
	float m_w, m_h;
	Gui::ToolTip *m_message;
	Gui::Image *m_face;
	Gui::Image *m_eyes;
	Gui::Image *m_nose;
	Gui::Image *m_hair;
	Gui::Image *m_mouth;
	Gui::Image *m_cloth;
	Gui::Image *m_extr1;
	Gui::Image *m_extr2;
};


void GenericChatForm::AddVideoWidget()
{
	Add(new FaceVideoLink(295,285), 5, 40);
	//Add(new DeadVideoLink(295,285), 5, 40);
	//AddFaceWidget();
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

