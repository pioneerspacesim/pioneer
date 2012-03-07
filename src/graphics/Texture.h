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
//    - Compare() to compare two arbitrary descriptors
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
	const TextureDescriptor &GetDescriptor() const { return m_descriptor; }

private:
	// only renderer (specifically, the cache in the baseclass) can create and
	// destroy these
	friend class Renderer;
	Texture(const TextureDescriptor &descriptor) : m_descriptor(descriptor) {}

	Texture(const Texture&);
	Texture &operator=(const Texture&);

	TextureDescriptor m_descriptor;
};

}

#endif
