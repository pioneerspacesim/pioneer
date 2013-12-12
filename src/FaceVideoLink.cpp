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
#include "FaceGenManager.h"

using namespace Graphics;

static const unsigned int FACE_WIDTH  = 295;
static const unsigned int FACE_HEIGHT = 285;

FaceVideoLink::FaceVideoLink(float w, float h, Uint32 flags, Uint32 seed,
	const std::string &name, const std::string &title) : VideoLink(w, h)
{
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip(0, Lang::VID_LINK_ESTABLISHED);

	if (!seed) seed = time(0);

	m_flags = flags;
	m_seed = seed;

	SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FACE_WIDTH, FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

	Sint8 gender=0;
	FaceGenManager::BlitFaceIm(faceim, gender, flags, seed);

	RefCountedPtr<Random> randPtr(new Random(seed));
	std::string charname = name;
	if (charname.empty())
		charname = Pi::luaNameGen->FullName((gender != 0), randPtr);

	m_characterInfo = new CharacterInfoText(w * 0.8f, h * 0.15f, charname, title);

	m_quad.reset(new Gui::TexturedQuad(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).CreateTexture(Gui::Screen::GetRenderer())));
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

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	m_quad->Draw(r, vector2f(0.0f), vector2f(size[0],size[1]));

	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
	r->Translate(0.f, size[1]- size[1] * 0.16f, 0.f);
	m_characterInfo->Draw();
}

void FaceVideoLink::DrawMessage() {
	float size[2];
	GetSize(size);

	float msgSize[2];
	m_message->GetSize(msgSize);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();
	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
	r->Translate(size[0]*0.5f-msgSize[0]*0.5f, size[1]-msgSize[1]*1.5f, 0);
	m_message->Draw();
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

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
		r->Translate((*i).pos[0], (*i).pos[1], 0);
		(*i).w->Draw();
	}
}
