#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "libs.h"

class Texture {
public:
	virtual ~Texture();

	virtual void Bind();
	virtual void Unbind();
	//perhaps also Bind(int) so you can switch active texture unit
	
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

protected:
	bool CreateFromSurface(SDL_Surface *s);
	bool CreateFromFile(const std::string &filename);

	struct TextureFormat {
		TextureFormat(GLenum format_, GLint internalFormat_, GLenum type_) :
			format(format_),
			internalFormat(internalFormat_),
			type(type_)
		{}
		GLenum format;        // GL_RGB, GL_RGBA...
		GLint internalFormat; // GL_RGB8, GL_RGB8_ALPHA8 etc.
		GLenum type;          // GL_UNSIGNED_BYTE etc.
	};

	enum WrapMode {
		REPEAT,
		CLAMP
	};

	enum FilterMode {
		NEAREST, //sharp
		LINEAR   //smooth (Texture will pick bilinear/trilinear, maybe anisotropic according to graphics settings)
	};

	Texture(GLenum target, const TextureFormat &format, WrapMode wrapMode, FilterMode filterMode, bool hasMipmaps) :
		m_target(target),
		m_format(format),
		m_wrapMode(wrapMode),
		m_filterMode(filterMode),
		m_hasMipmaps(hasMipmaps),
		m_width(-1),
		m_height(-1)
	{}

	GLenum m_target; // GL_TEXTURE2D etc.
	
	TextureFormat m_format;
	WrapMode m_wrapMode;
	FilterMode m_filterMode;
	bool m_hasMipmaps;

	int m_width;
	int m_height;

	GLuint m_glTexture;
};


class ModelTexture : public Texture {
public:
	ModelTexture(const std::string &filename, bool preload = false);

	virtual void Bind() {
		if (!m_isLoaded)
			Load();
		Texture::Bind();
	}

	const std::string &GetFilename() const { return m_filename; }

private:
	void Load();

	std::string m_filename;

	bool m_isLoaded;
};


class UITexture : public Texture {
public:
	UITexture(SDL_Surface *s);
	UITexture(const std::string &filename);
};

#endif
