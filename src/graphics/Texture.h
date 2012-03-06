#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "Renderer.h"
#include "vector2.h"

namespace Graphics {

// There's two classes main classes
//
// - TextureDescriptor provides a "name" for a texture. It effectively contains
//   all the information needed to describe a texture (type, format, build
//   options, and any subclass-specific data), but does not contain any actual
//   texture data. It exists primarily to be used as a way to identify items in
//   the texture cache.
//
//   The base TextureDescriptor class cannot be instantiated directly.
//   Subclasses should be created for specific use cases.
//
//   TextureDescriptor subclasses must provide two methods:
//
//    - GetData() returns a heap-allocated TextureDescriptor::Data object
//      containing a pointer to the raw data and the dimensions.
//    
//    - IsEqual() to determine if two descriptors are the same (used by
//      operator=)
//
// - Texture encapsulates the details of a fully-instantiated texture. The
//   itself holds very little useful data - just the data and texture
//   dimensions, as well as the renderer-specific RenderInfo. There can only
//   ever be one of these per value of TextureDescriptor. They are always
//   obtained from the cache.



// object describing a single texture. two textures that compare the same are
// assumed to be completely interchangable
class Texture : public Renderable {
public:
	// returns the data (pixel) size of the texture. this usually corresponds
	// to the size of the data that was used to create the texture (eg the
	// on-disk image file). if this texture's data has not been requested yet,
	// the size will be 0
	vector2f GetSize() const { return m_size; }

	// return the texel size of the texture. this will typically be
	// [1.0f,1.0f] but might not be if the texture has been resized (eg for
	// power-of-two restrictions). if this texture's data has not been
	// requested yet, the size will be 0
	vector2f GetTextureSize() const { return m_texSize; }

	// bind/unbind the texture to the currently active texture unit
	// XXX DEPRECATED remove when LMR starts using the renderer
	//virtual void Bind();
	//virtual void Unbind();

private:
	// only renderer (specifically, the cache in the baseclass) can create and
	// destroy these
	friend class Renderer;
	Texture(const vector2f &size, const vector2f &texSize) : m_size(size), m_texSize(texSize) {}
	virtual ~Texture();

	Texture(const Texture&);
	Texture &operator=(const Texture&);

	vector2f m_size;
	vector2f m_texSize;

};

}

#endif
