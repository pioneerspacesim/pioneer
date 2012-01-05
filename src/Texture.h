#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "libs.h"

class Texture {
public:
	virtual ~Texture();

	virtual void Bind();
	virtual void Unbind();
	//perhaps also Bind(int) so you can switch active texture unit
	
	GLenum GetTarget() const { return m_target; }
	
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	float GetTextureWidth() const { return m_texWidth; }
	float GetTextureHeight() const { return m_texHeight; }

	void DrawQuad(float x, float y, float w, float h);

protected:
	void CreateFromArray(const void *data, int width, int height);
	bool CreateFromSurface(SDL_Surface *s);
	bool CreateFromFile(const std::string &filename);

	struct TextureFormat {
		TextureFormat(GLint internalFormat_, GLenum dataFormat_, GLenum dataType_) :
			internalFormat(internalFormat_),
			dataFormat(dataFormat_),
			dataType(dataType_)
		{}
		GLint internalFormat; // GL_RGB8, GL_RGB8_ALPHA8 etc.
		GLenum dataFormat;    // GL_RGB, GL_RGBA...
		GLenum dataType;      // GL_UNSIGNED_BYTE etc.
	};

	enum WrapMode {
		REPEAT,
		CLAMP
	};

	enum FilterMode {
		NEAREST, //sharp
		LINEAR   //smooth (Texture will pick bilinear/trilinear, maybe anisotropic according to graphics settings)
	};

	Texture(GLenum target, const TextureFormat &format, WrapMode wrapMode, FilterMode filterMode, bool hasMipmaps, bool wantPow2Resize) :
		m_target(target),
		m_format(format),
		m_wrapMode(wrapMode),
		m_filterMode(filterMode),
		m_hasMipmaps(hasMipmaps),
		m_wantPow2Resize(wantPow2Resize),
		m_width(-1),
		m_height(-1),
		m_texWidth(0.0f),
		m_texHeight(0.0f),
		m_glTexture(0)
	{}

	GLenum m_target; // GL_TEXTURE2D etc.
	
	TextureFormat m_format;
	WrapMode m_wrapMode;
	FilterMode m_filterMode;
	bool m_hasMipmaps;
	bool m_wantPow2Resize;

	int m_width;
	int m_height;

	float m_texWidth;
	float m_texHeight;

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
