#ifndef _TEXTUREDESCRIPTOR_H
#define _TEXTUREDESCRIPTOR_H

#include <SDL.h>
#include <string>
#include "vector2.h"

namespace Graphics {

class TextureDescriptor {
public:

	// descriptor types. needed for comparison of arbitrary descriptors
	enum Type {
		TYPE_GUISURFACE,
		TYPE_GUIFILE,
		TYPE_GUIGRADIENT,
		TYPE_GLYPH,
		TYPE_WORLD
	};

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

		friend bool operator<(const Format &a, const Format &b) {
			return (a.internalFormat < b.internalFormat && a.dataFormat < b.dataFormat && a.dataType < b.dataType);
		}
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

		friend bool operator<(const Options &a, const Options &b) {
			return (a.wrapMode < b.wrapMode && a.filterMode < b.filterMode && a.mipmaps < b.mipmaps);
		}
	};

	// raw texture data
	struct Data {
		Data(const void *_data, const vector2f &_dataSize, const vector2f &_texSize = vector2f(1.0f)) : data(_data), dataSize(_dataSize), texSize(_texSize) {}
		virtual ~Data() {}
		const void *data;     // raw texture data. format is defined elsewhere
		vector2f    dataSize; // width/height of the raw data
		vector2f    texSize;  // width/height of the "usable" texels (eg after POT-extension)
	};

protected:
	TextureDescriptor(Type _type, Target _target, Format _format, Options _options) : type(_type), target(_target), format(_format), options(_options) {}

public:
	const Type type;
	const Target target;
	const Format format;
	const Options options;

	// get the raw texture data. allocate with new, as the caller will delete
	// when done. this will usually only be called by the renderer, and it
	// will never call this more than once, so you should not preload this
	// data and you need not cache it for further calls
	// non-renderer callers should probably not use this.
	virtual const Data *GetData() const { return 0; }

	// return true if this texture is "less than" the passed-in texture
	// - baseclass Compare
	// - construction args
    // this is used for cache lookup, so the ordering is important
	virtual bool Compare(const TextureDescriptor &b) const {
		return (type < b.type && target < b.target && format < b.format && options < b.options);
	}

	// create a copy of this. should usually just invoke the copy constructor
	virtual TextureDescriptor *Clone() const {
		return new TextureDescriptor(*this);
	}

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
};

}

#endif
