#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "libs.h"

/*
 * Texture is a class to manage the details of a single texture. you can't
 * instantiate this class directly. most of the time you'll want either
 * UITexture or ModelTexture, which will create an appropriate texture for
 * general UI or world drawing use.
 */
class Texture {
public:

	// texture format definition. holds details of how the texture is stored
	// internally in GL and what the incoming data looks like. see the docs
	// for glTexImage* for details. You don't need to worry about this unless
	// you're subclassing Texture.
	struct Format {
		Format(GLint internalFormat_, GLenum dataFormat_, GLenum dataType_) :
			internalFormat(internalFormat_),
			dataFormat(dataFormat_),
			dataType(dataType_)
		{}
		GLint internalFormat; // GL_RGB8, GL_RGB8_ALPHA8 etc.
		GLenum dataFormat;    // GL_RGB, GL_RGBA...
		GLenum dataType;      // GL_UNSIGNED_BYTE etc.
	};

	// wrap mode. decides what to do when the texture is not large enough to
	// cover the mesh. ignore unless subclassing.
	enum WrapMode {
		REPEAT, // GL_REPEAT
		CLAMP   // GL_CLAMP_TO_EDGE
	};

    // filter mode. decides how texture elements are mapped to the mesh.
    // ignore unless subclassing.
	enum FilterMode {
		NEAREST, // GL_NEAREST, sharp
		LINEAR   // GL_LINEAR, smooth
	};

	virtual ~Texture();

	// bind/unbind the texture to the currently active texture unit
	virtual void Bind();
	virtual void Unbind();

	// see if the texture has an underlying GL texture yet. allows subclasses
	// to support on-demand texture loading. Bind() will assert if IsCreated()
	// is false.
	bool IsCreated() const { return m_glTexture != 0; }
	
	// get the texture target, eg GL_TEXTURE_2D. set by the subclass
	GLenum GetTarget() const { return m_target; }
	
	// return the pixel height/width of the texture. this usually corresponds
	// to the size of the data that was used to create the texture (eg the
	// on-disk image file)
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	// return the texel height/width of the texture. this will typically be
	// [1.0f,1.0f] but might not be if the texture has been resized (eg for
	// power-of-two restrictions)
	float GetTextureWidth() const { return m_texWidth; }
	float GetTextureHeight() const { return m_texHeight; }

	// return the Texture::Format definition of this texture. useful if you
	// need to know the underlying texture format
	const Format &GetFormat() const { return m_format; }

	// convenience methods to draw a quad using this texture. it will enable
	// the target and bind the texture, draw the quad and then unbind the
	// texture and disbale the target. x/y/w/h are the position and size of
	// the quad, tx/ty/tw/th are the texel position/size of the texture.
	void DrawQuad(float x, float y, float w, float h, float tx, float ty, float tw, float th);
	inline void DrawQuad(float x, float y, float w, float h) {
		DrawQuad(x, y, w, h, 0, 0, GetTextureWidth(), GetTextureHeight());
	}
	inline void DrawQuad(float w, float h) {
		DrawQuad(0, 0, w, h, 0, 0, GetTextureWidth(), GetTextureHeight());
	}

	// like DrawQuad, but for drawing the quad for the UI. Pioneer's UI is
	// inverted so that the y-coord goes down the screen instead of up. this
	// draws the quad with opposite winding so it does the right thing.
	void DrawUIQuad(float x, float y, float w, float h, float tx, float ty, float tw, float th);
	inline void DrawUIQuad(float x, float y, float w, float h) {
		DrawUIQuad(x, y, w, h, 0, 0, GetTextureWidth(), GetTextureHeight());
	}
	inline void DrawUIQuad(float w, float h) {
		DrawUIQuad(0, 0, w, h, 0, 0, GetTextureWidth(), GetTextureHeight());
	}

protected:

	// constructor for subclasses. if wantMipmaps is true then mipmaps will be
	// generated when the texture is created.
	Texture(GLenum target, const Format &format, WrapMode wrapMode, FilterMode filterMode, bool wantMipmaps) :
		m_target(target),
		m_format(format),
		m_wrapMode(wrapMode),
		m_filterMode(filterMode),
		m_wantMipmaps(wantMipmaps),
		m_width(0),
		m_height(0),
		m_texWidth(0.0f),
		m_texHeight(0.0f),
		m_glTexture(0)
	{}

	// create the underlying texture from raw data. data is expected to be of
	// the format and type setup by the subclass in its Texture::Format
	void CreateFromArray(const void *data, unsigned int width, unsigned int height);

	// create the texture from a SDL surface. this method is designed for 24
	// and 32-bit colour surfaces, and won't work properly if the incoming
	// data form is something else.
	//
	// if forceRGBA is true, incoming surfaces will be converted to RGBA
	// before being passed to CreateFromArray(). if its false, then 24-bit
	// surfaces will be converted to RGB and 32-bit to RGBA and the internal
	// data format will be adjusted appropriately. this can save texture
	// memory but doesn't always do the right thing if the user expects the
	// texture to always have an alpha channel (eg for UI textures)
	bool CreateFromSurface(SDL_Surface *s, bool forceRGBA = true);

	// loads the given file into a SDL surface and passes the result to
	// CreateFromSurface()
	bool CreateFromFile(const std::string &filename, bool forceRGBA = true);

	// get the GL texture name. don't use this if you just want to bind the
	// texture, use Bind() for that.
	GLuint GetGLTexture() const { return m_glTexture; }

private:
	// textures should not be copied as they have shared GL state
	Texture(const Texture &) : m_format(0,0,0) {}

	void DrawQuadArray(const GLfloat *array);

	GLenum m_target;
	
	Format m_format;
	WrapMode m_wrapMode;
	FilterMode m_filterMode;
	bool m_wantMipmaps;
	bool m_wantPow2Resize;

	unsigned int m_width;
	unsigned int m_height;

	float m_texWidth;
	float m_texHeight;

	GLuint m_glTexture;
};


// subclass for model textures. primarily allows lazy-loaded textures, where
// they aren't pulled from disk until the first call to Bind().
class ModelTexture : public Texture {
public:
	ModelTexture(const std::string &filename, bool preload = false);

	virtual void Bind() {
		if (!IsCreated())
			Load();
		Texture::Bind();
	}

	const std::string &GetFilename() const { return m_filename; }

private:
	void Load();

	std::string m_filename;
};


// subclass for UI textures. these can be constructed directly from a SDL
// surface or loaded from disk
class UITexture : public Texture {
public:
	UITexture(SDL_Surface *s);
	UITexture(const std::string &filename);
};

#endif
