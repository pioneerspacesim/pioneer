#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include "RefCounted.h"
#include "Renderer.h"

namespace Graphics {

/*
 * Texture is a class to manage the details of a single texture. you can't
 * instantiate this class directly. most of the time you'll want either
 * UITexture or ModelTexture, which will create an appropriate texture for
 * general UI or world drawing use.
 */
class Texture : public RefCounted, public Renderable {
public:

	// texture type. ignore unless subclassing
	// XXX this is possibly too GL-centric
	enum Target {
		TARGET_2D
	};

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

	// various options that influence how the texture is created
	struct Options {

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

		Options(WrapMode _wrapMode, FilterMode _filterMode, bool _mipmaps):
			wrapMode(_wrapMode),
			filterMode(_filterMode),
			mipmaps(_mipmaps)
		{}

		WrapMode wrapMode;
		FilterMode filterMode;
		bool mipmaps;
	};

	// texture properties
	Target GetTarget() const { return m_target; }
	const Format &GetFormat() const { return m_format; }
	const Options &GetOptions() const { return m_options; }

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

	// bind/unbind the texture to the currently active texture unit
	// XXX DEPRECATED remove when LMR starts using the renderer
	virtual void Bind();
	virtual void Unbind();

protected:

	// constructor for subclasses. if wantMipmaps is true then mipmaps will be
	// generated when the texture is created.
	Texture(Target target, const Format &format, const Options &options) :
		m_target(target),
		m_format(format),
		m_options(options),
		m_width(0),
		m_height(0),
		m_texWidth(1.0f),
		m_texHeight(1.0f)
	{}

	// create the underlying texture from raw data. data is expected to be of
	// the format and type setup by the subclass in its
	bool CreateFromArray(Renderer *r, const void *data, unsigned int width, unsigned int height);

	// create the texture from a SDL surface. this method is designed for 24
	// and 32-bit colour surfaces, and won't work properly if the incoming
	// data form is something else.
	//
	// if potExtend is true, incoming surfaces will be extended to
	// power-of-two dimensions. this will break UV coordinates; use
	// GetTextureWidth() and GetTextureHeight() to scale them if necessary
	//
	// if forceRGBA is true, incoming surfaces will be converted to RGBA
	// before being passed to CreateFromArray(). if its false, then 24-bit
	// surfaces will be converted to RGB and 32-bit to RGBA and the internal
	// data format will be adjusted appropriately. this can save texture
	// memory but doesn't always do the right thing if the user expects the
	// texture to always have an alpha channel (eg for UI textures)
	bool CreateFromSurface(Renderer *r, SDL_Surface *s, bool potExtend = false, bool forceRGBA = true);

	// loads the given file into a SDL surface and passes the result to
	// CreateFromSurface()
	bool CreateFromFile(Renderer *r, const std::string &filename, bool potExtend = false, bool forceRGBA = true);

private:
	// textures should not be copied as they have shared GL state
	Texture(const Texture &);

	Target m_target;
	Format m_format;
	Options m_options;

	unsigned int m_width;
	unsigned int m_height;

	float m_texWidth;
	float m_texHeight;
};

}

#endif
