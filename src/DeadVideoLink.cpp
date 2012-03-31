#include "DeadVideoLink.h"
#include "Pi.h"
#include "Lang.h"

static const int textureSize = 512;

DeadVideoLink::DeadVideoLink(float w, float h) : VideoLink(w, h)
{
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip(Lang::VID_LINK_DOWN);

	Graphics::TextureDescriptor descriptor(Graphics::TEXTURE_RGB, vector2f(textureSize), Graphics::LINEAR_CLAMP);
	m_texture.Reset(Gui::Screen::GetRenderer()->CreateTexture(descriptor));
	m_quad.Reset(new Gui::TexturedQuad(m_texture.Get()));

	UpdateWhiteNoise();
}	

DeadVideoLink::~DeadVideoLink()
{
	delete m_message;
}

void DeadVideoLink::Draw()
{
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

	m_message->SetText(Lang::VID_LINK_DOWN);

	UpdateWhiteNoise();

	m_quad->Draw(Gui::Screen::GetRenderer(), vector2f(0.0f), vector2f(size[0],size[1]));
	DrawMessage();
}

void DeadVideoLink::DrawMessage()
{
	float size[2];
	float msgSize[2];
	GetSize(size);
	m_message->GetSize(msgSize);
	glPushMatrix();
	glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
	m_message->Draw();
	glPopMatrix();
}

void DeadVideoLink::UpdateWhiteNoise()
{
	Uint32 noise[textureSize*textureSize*4];
	for (unsigned int i=0; i<textureSize*textureSize; i++) {
		Uint8 b = Pi::rng.Int32() & 0xff;
		noise[i] = b<<24|b<<16|b<<8|b;
	}
	m_texture->Update(noise, vector2f(textureSize), Graphics::IMAGE_RGB, Graphics::IMAGE_UNSIGNED_BYTE);
}
