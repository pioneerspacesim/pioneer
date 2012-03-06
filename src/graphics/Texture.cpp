#include "Texture.h"
#include <SDL_image.h>
#include <cassert>
#include "utils.h"

namespace Graphics {

#if 0
// XXX copied from RendererLegacy. delete when Bind and Unbind go away (ie
// when LMR stops using them)
struct TextureRenderInfo : public RenderInfo {
	TextureRenderInfo();
	virtual ~TextureRenderInfo();
	GLenum target;
	GLuint texture;
};
void Texture::Bind()
{
	TextureRenderInfo *textureInfo = static_cast<TextureRenderInfo*>(GetRenderInfo());
	glEnable(textureInfo->target);
	glBindTexture(textureInfo->target, textureInfo->texture);
}
void Texture::Unbind()
{
	TextureRenderInfo *textureInfo = static_cast<TextureRenderInfo*>(GetRenderInfo());
	glBindTexture(textureInfo->target, 0);
	glDisable(textureInfo->target);
}
#endif

}
