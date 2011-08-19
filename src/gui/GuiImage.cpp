#include "libs.h"
#include "GuiImage.h"

namespace Gui {

Image::~Image()
{
	glDeleteTextures(1, &m_tex);
}

Image::Image(const char *filename): Widget()
{
	m_col[0] = m_col[1] = m_col[2] = m_col[3] = 1.0f;
	SDL_Surface *is = IMG_Load(filename);
	if (!is) {
		fprintf(stderr, "Could not load %s\n", filename);
		exit(0);
	}
	m_imgw = is->w;
	m_imgh = is->h;

	SetSize(float(m_imgw), float(m_imgh));

	/* gl textures must be POT, dim > 64 */
	int texw, texh;
	{
		int nbit = 0;
		int sz = m_imgw;
		while (sz) { sz >>= 1; nbit++; }
		texw = std::max(64, 1<<nbit);

		sz = m_imgh;
		nbit = 0;
		while (sz) { sz >>= 1; nbit++; }
		texh = std::max(64, 1<<nbit);
	}
	m_invtexw = 1.0f / texw;
	m_invtexh = 1.0f / texh;

	SDL_Rect src,dest;
	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, texw, texh, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
	// just want to directly copy RGBA values, not blend using alpha
	SDL_SetAlpha(s, 0, 0);
	SDL_SetAlpha(is, 0, 0);
	SDL_BlitSurface(is, NULL, s, NULL);
	/* a silly workaround for the GL_LINEAR filtering of the texture, which leads to the black bits of the texture outside the actual turd leaking in via bilinear filtering.. */
	dest.x = m_imgw; dest.y = 0; dest.w = m_imgw; dest.h = m_imgh;
	src.x = m_imgw-1; src.y = 0; src.w=1; src.h = m_imgh;
	SDL_BlitSurface(is, &src, s, &dest);

	dest.x = 0; dest.y = m_imgh; dest.w = m_imgw; dest.h = m_imgh;
	src.x = 0; src.y = m_imgh-1; src.w=m_imgw; src.h = 1;
	SDL_BlitSurface(is, &src, s, &dest);

	SDL_FreeSurface(is);

	glEnable (GL_TEXTURE_2D);
	glGenTextures (1, &m_tex);
	glBindTexture (GL_TEXTURE_2D, m_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texw, texh, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable (GL_TEXTURE_2D);

	SDL_FreeSurface(s);
}

void Image::GetSizeRequested(float size[2])
{
	size[0] = float(m_imgw);
	size[1] = float(m_imgh);
}

void Image::SetModulateColor(float r, float g, float b, float a)
{
	m_col[0] = r;
	m_col[1] = g;
	m_col[2] = b;
	m_col[3] = a;
}

void Image::Draw()
{
	float allocSize[2];
	GetSize(allocSize);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	if ((m_col[0] >= 1) && (m_col[1] >= 1) && (m_col[2] >= 1) && (m_col[3] >= 1)) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4fv(m_col);
	}
	glBegin(GL_QUADS);
		float w = m_imgw * m_invtexw;
		float h = m_imgh * m_invtexh;
		glTexCoord2f(0,h);
		glVertex2f(0,allocSize[1]);
		glTexCoord2f(w,h);
		glVertex2f(allocSize[0],allocSize[1]);
		glTexCoord2f(w,0);
		glVertex2f(allocSize[0],0);
		glTexCoord2f(0,0);
		glVertex2f(0,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}


}

