#include "Renderer.h"
#include "Texture.h"
#include "TextureDescriptor.h"

namespace Graphics {

Renderer::Renderer(int w, int h) :
	m_width(w), m_height(h)
{

}

Renderer::~Renderer()
{

}

Texture *Renderer::AddCachedTexture(const TextureDescriptor *descriptor)
{
	ScopedPtr<const TextureDescriptor::Data> texData(descriptor->GetData());
	Texture *texture = new Texture(texData->dataSize, texData->texSize);
	assert(CreateTexture(texture, descriptor, const_cast<void*>(texData->data), texData->dataSize));
	m_textures.insert(std::pair<TextureDescriptor const*,Texture*>(descriptor->Clone(),texture));
	return texture;
}

}
