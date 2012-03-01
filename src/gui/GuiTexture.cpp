#include "GuiTexture.h"

namespace Gui {

Texture::Texture(SDL_Surface *s) :
    Graphics::Texture(Texture::TARGET_2D, Format(Texture::Format::INTERNAL_RGBA, Texture::Format::DATA_RGBA, Texture::Format::DATA_UNSIGNED_BYTE), CLAMP, LINEAR, false, true)
{
	CreateFromSurface(s);
}

Texture::Texture(const std::string &filename) :
    Graphics::Texture(Texture::TARGET_2D, Format(Texture::Format::INTERNAL_RGBA, Texture::Format::DATA_RGBA, Texture::Format::DATA_UNSIGNED_BYTE), CLAMP, LINEAR, false, true)
{
	CreateFromFile(filename);
}

}
