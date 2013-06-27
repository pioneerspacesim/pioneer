// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FaceVideoLink.h"
#include "Lang.h"
#include "Pi.h"
#include "LuaNameGen.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"
#include "FileSystem.h"
#include "SDLWrappers.h"

using namespace Graphics;

static const unsigned int FACE_WIDTH  = 295;
static const unsigned int FACE_HEIGHT = 285;

// XXX these shouldn't really be hardcoded. it'd be much nicer to poke through
// the facegen/ dir and figure out what we've got available. that or some
// config file
static const unsigned int MAX_HEAD  = 20;
static const unsigned int MAX_EYES  = 20;
static const unsigned int MAX_NOSE  = 20;
static const unsigned int MAX_MOUTH = 20;
static const unsigned int MAX_HAIR  = 20;

static const unsigned int MAX_CLOTHES     = 20;
static const unsigned int MAX_ARMOUR      = 3;
static const unsigned int MAX_ACCESSORIES = 16;

static const unsigned int MAX_BACKGROUND = 18;

static void _blit_image(SDL_Surface *s, const char *filename, int xoff, int yoff)
{
	SDLSurfacePtr is = LoadSurfaceFromFile(filename);
	// XXX what should this do if the image couldn't be loaded?
	if (! is) { return; }

	SDL_Rect destrec = { 0, 0, 0, 0 };
	destrec.x = ((FACE_WIDTH-is->w-1)/2)+xoff;
	destrec.y = yoff;
	SDL_BlitSurface(is.Get(), 0, s, &destrec);
}

static void _blit_image(const SDLSurfacePtr &s, const char *filename, int xoff, int yoff)
{
	_blit_image(s.Get(), filename, xoff, yoff);
}

FaceVideoLink::FaceVideoLink(float w, float h, Uint32 flags, Uint32 seed,
	const std::string &name, const std::string &title) : VideoLink(w, h)
{
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip(0, Lang::VID_LINK_ESTABLISHED);

	if (!seed) seed = time(0);

	RefCountedPtr<Random> rand(new Random(seed));

	m_flags = flags;
	m_seed = seed;

	int race = rand->Int32(0,2);

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
			gender = rand->Int32(0,1);
			break;
	}

	std::string charname = name;
	if (charname.empty())
		charname = Pi::luaNameGen->FullName((gender != 0), rand);

	m_characterInfo = new CharacterInfoText(w * 0.8f, h * 0.15f, charname, title);

	int head  = rand->Int32(0,MAX_HEAD);
	int eyes  = rand->Int32(0,MAX_EYES);
	int nose  = rand->Int32(0,MAX_NOSE);
	int mouth = rand->Int32(0,MAX_MOUTH);
	int hair  = rand->Int32(0,MAX_HAIR);

	int clothes = rand->Int32(0,MAX_CLOTHES);

	int armour = rand->Int32(0,MAX_ARMOUR);

	int accessories = rand->Int32(0,MAX_ACCESSORIES);

	int background = rand->Int32(0,MAX_BACKGROUND);

	char filename[1024];

	SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FACE_WIDTH, FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

	snprintf(filename, sizeof(filename), "facegen/backgrounds/background_%d.png", background);
	_blit_image(faceim, filename, 0, 0);

	snprintf(filename, sizeof(filename), "facegen/race_%d/head/head_%d_%d.png", race, gender, head);
	_blit_image(faceim, filename, 0, 0);

	if (!(flags & ARMOUR)) {
		snprintf(filename, sizeof(filename), "facegen/clothes/cloth_%d_%d.png", gender, clothes);
		_blit_image(faceim, filename, 0, 135);
	}

	snprintf(filename, sizeof(filename), "facegen/race_%d/eyes/eyes_%d_%d.png", race, gender, eyes);
	_blit_image(faceim, filename, 0, 41);

	snprintf(filename, sizeof(filename), "facegen/race_%d/nose/nose_%d_%d.png", race, gender, nose);
	_blit_image(faceim, filename, 1, 89);

	snprintf(filename, sizeof(filename), "facegen/race_%d/mouth/mouth_%d_%d.png", race, gender, mouth);
	_blit_image(faceim, filename, 0, 155);

	if (!(flags & ARMOUR)) {
		snprintf(filename, sizeof(filename), "facegen/accessories/acc_%d.png", accessories);
		if (rand->Int32(0,1)>0)	_blit_image(faceim, filename, 0, 0);

		snprintf(filename, sizeof(filename), "facegen/race_%d/hair/hair_%d_%d.png", race, gender, hair);
		_blit_image(faceim, filename, 0, 0);
	}
	else {
		snprintf(filename, sizeof(filename), "facegen/clothes/armour_%d.png", armour);
		_blit_image(faceim, filename, 0, 0);
	}

	m_quad.Reset(new Gui::TexturedQuad(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).CreateTexture(Gui::Screen::GetRenderer())));
}

FaceVideoLink::~FaceVideoLink() {
	delete m_message;
	delete m_characterInfo;
}

void FaceVideoLink::Draw() {
	float size[2];
	GetSize(size);

	Uint32 now = SDL_GetTicks();

	if (now - m_created < 1500) {
		glBegin(GL_QUADS);
			glColor3f(0,0,0);
			glVertex2f(0,0);
			glVertex2f(0,size[1]);
			glVertex2f(size[0],size[1]);
			glVertex2f(size[0],0);
		glEnd();

		m_message->SetText(Lang::VID_CONNECTING);
		DrawMessage();

		return;
	}

	m_quad->Draw(Gui::Screen::GetRenderer(), vector2f(0.0f), vector2f(size[0],size[1]));

	glPushMatrix();
	glTranslatef(0.f, size[1]- size[1] * 0.16f, 0.f);
	m_characterInfo->Draw();
	glPopMatrix();
}

void FaceVideoLink::DrawMessage() {
	float size[2];
	GetSize(size);

	float msgSize[2];
	m_message->GetSize(msgSize);

	glPushMatrix();
	glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]-msgSize[1]*1.5f, 0);
	m_message->Draw();
	glPopMatrix();
}

CharacterInfoText::CharacterInfoText(float w, float h,
	const std::string &name, const std::string &title) :
	Gui::Fixed(w, h),
	m_characterName(name),
	m_characterTitle(title),
	m_width(w),
	m_height(h)
{
	if (m_characterTitle.empty())
		h = h/1.5f;
	SetSize(w, h);
	m_background = new Gui::Gradient(w, h, Color(0.1f, 0.1f, 0.1f, 0.8f),
		Color(0.f, 0.f, 0.1f, 0.f), Gui::Gradient::HORIZONTAL);
	Gui::Screen::PushFont("OverlayFont");
	m_nameLabel = new Gui::Label(m_characterName);
	m_titleLabel = new Gui::Label(m_characterTitle);
	Gui::Screen::PopFont();
	Add(m_background, 0.f, 0.f);
	Add(m_nameLabel, 25.f, 5.f);
	Add(m_titleLabel, 25.f, m_height/2);
}

CharacterInfoText::~CharacterInfoText()
{

}

void CharacterInfoText::SetCharacterName(const std::string &name)
{
	m_characterName = name;
	m_nameLabel->SetText(name);
	UpdateAllChildSizes();
}

void CharacterInfoText::SetCharacterTitle(const std::string &title)
{
	m_characterTitle = title;
	m_titleLabel->SetText(title);
	UpdateAllChildSizes();
}

void CharacterInfoText::GetSizeRequested(float size[2])
{
	size[0] = m_width;
	size[1] = m_height;
}

void CharacterInfoText::Draw()
{
	if (m_characterName.empty() && m_characterTitle.empty()) return;

	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		glPushMatrix();
		glTranslatef((*i).pos[0], (*i).pos[1], 0);
		(*i).w->Draw();
		glPopMatrix();
	}
}
