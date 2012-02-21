#include "GuiTexture.h"

namespace Gui {

Texture::Texture(SDL_Surface *s) :
    Graphics::Texture(GL_TEXTURE_2D, Format(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), CLAMP, LINEAR, false)
{
	CreateFromSurface(s);
}

Texture::Texture(const std::string &filename) :
    Graphics::Texture(GL_TEXTURE_2D, Format(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), CLAMP, LINEAR, false)
{
	CreateFromFile(filename);
}

}
