#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "libs.h"

class Texture {
public:
	Texture(const std::string &filename, bool preload = false);
	Texture(SDL_Surface *s);
	~Texture();

	void BindTexture();

	const std::string &GetFilename() const { return m_filename; }
	bool IsLoaded() const { return m_isLoaded; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

private:
	void Load();

	bool CreateFromSurface(SDL_Surface *s);

	std::string m_filename;
	bool m_isLoaded;
	int m_width, m_height;
	GLuint m_tex;
};

#endif
