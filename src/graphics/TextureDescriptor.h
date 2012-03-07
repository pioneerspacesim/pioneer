#ifndef _TEXTUREDESCRIPTOR_H
#define _TEXTUREDESCRIPTOR_H

#include <SDL.h>
#include <string>
#include "vector2.h"

namespace Graphics {

class TextureDescriptor {
public:

	// data format
	struct Format {

		// internal data format
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

	// constructor
	TextureDescriptor(const Format &_format, const Options &_options, const vector2f &_dataSize, const vector2f &_texSize = vector2f(1.0f)) :
		format(_format), options(_options), dataSize(_dataSize), texSize(_texSize)
	{}

	const Format format;
	const Options options;

	// width/height of the raw data array underneath the texture (eg pixels)
	const vector2f dataSize;

	// width/height of texture in texels
	const vector2f texSize;
};


#if 0
protected:

	// helpers for GetData()

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
	const Data *GetDataFromSurface(SDL_Surface *s, bool potExtend = false, bool forceRGBA = true) const;

	// loads the given file into a SDL surface and passes the result to
	// CreateFromSurface()
	const Data *GetDataFromFile(const std::string &filename, bool potExtend = false, bool forceRGBA = true) const;

private:
	const Data *GetDataFromSurfaceInternal(SDL_Surface *s, bool potExtend, bool forceRGBA, bool freeSurface) const;
#endif

}

#endif
