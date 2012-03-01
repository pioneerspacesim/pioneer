#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include "RefCounted.h"

namespace Graphics {

/*
 * Texture is a class to manage the details of a single texture. you can't
 * instantiate this class directly. most of the time you'll want either
 * UITexture or ModelTexture, which will create an appropriate texture for
 * general UI or world drawing use.
 */
class Texture : public RefCounted {
public:

	// texture format definition. holds details of how the texture is stored
	// internally in GL and what the incoming data looks like. see the docs
	// for glTexImage* for details. You don't need to worry about this unless
	// you're subclassing Texture.
	struct Format {

		// internal data format
		// XXX advisory only. not even required? hmm.
		enum InternalFormat {
			INTERNAL_RGBA,
			INTERNAL_RGB,
			INTERNAL_LUMINANCE_ALPHA
		};

		// incoming data format
		enum DataFormat {
			DATA_RGBA,
			DATA_RGB,
			DATA_LUMINANCE_ALPHA
		};

		// incoming data type
		enum DataType {
			DATA_UNSIGNED_BYTE,
			DATA_FLOAT
		};

		Format(InternalFormat internalFormat_, DataFormat dataFormat_, DataType dataType_) :
			internalFormat(internalFormat_),
			dataFormat(dataFormat_),
			dataType(dataType_)
		{}
		InternalFormat internalFormat;
		DataFormat dataFormat;
		DataType dataType;
	};

	// texture type. ignore unless subclassing
	// XXX this is possibly too GL-centric
	enum Target {
		TARGET_1D,
		TARGET_2D
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
	Target GetTarget() const { return m_target; }
	
	// return the pixel height/width of the texture. this usually corresponds
	// to the size of the data that was used to create the texture (eg the
	// on-disk image file)
	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }

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
	Texture(Target target, const Format &format, WrapMode wrapMode, FilterMode filterMode, bool wantMipmaps, bool wantPow2Resize = false) :
		m_target(target),
		m_format(format),
		m_wrapMode(wrapMode),
		m_filterMode(filterMode),
		m_wantMipmaps(wantMipmaps),
		m_wantPow2Resize(wantPow2Resize),
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
	Texture(const Texture &);

	void DrawQuadArray(const GLfloat *array);

	Target m_target;
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

}

#endif
