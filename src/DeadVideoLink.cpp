#include "DeadVideoLink.h"
#include "Pi.h"
#include "Lang.h"

#define TEXSIZE	512

DeadVideoLink::DeadVideoLink(float w, float h) : VideoLink(w, h) {
	m_created = SDL_GetTicks();
	m_message = new Gui::ToolTip(Lang::VID_LINK_DOWN);
	glEnable (GL_TEXTURE_2D);
	glGenTextures (1, &m_tex);
	glBindTexture (GL_TEXTURE_2D, m_tex);
	PutRandomCrapIntoTexture();
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable (GL_TEXTURE_2D);
}	

DeadVideoLink::~DeadVideoLink() {
	glDeleteTextures(1, &m_tex);
	delete m_message;
}

void DeadVideoLink::Draw() {
	float size[2]; GetSize(size);
	if (SDL_GetTicks() - m_created < 1500) {
		m_message->SetText(Lang::VID_CONNECTING);
		glBegin(GL_QUADS);
			glColor3f(0,0,0);
			glVertex2f(0,0);
			glVertex2f(0,size[1]);
			glVertex2f(size[0],size[1]);
			glVertex2f(size[0],0);
		glEnd();
		DrawMessage();
	} else {
		m_message->SetText(Lang::VID_LINK_DOWN);

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

void DeadVideoLink::DrawMessage() {
	float size[2];
	float msgSize[2];
	GetSize(size);
	m_message->GetSize(msgSize);
	glPushMatrix();
	glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
	m_message->Draw();
	glPopMatrix();
}

void DeadVideoLink::PutRandomCrapIntoTexture() {
	int *randcrap = static_cast<int*>(alloca(TEXSIZE*TEXSIZE));
	for (unsigned int i=0; i<TEXSIZE*TEXSIZE/sizeof(int); i++) randcrap[i] = (Pi::rng.Int32() & 0xfcfcfcfc) >> 2;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, randcrap);
}
