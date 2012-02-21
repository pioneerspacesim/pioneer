#ifndef _GUITEXTURE_H
#define _GUITEXTURE_H

#include "graphics/Texture.h"

namespace Gui {

// subclass for UI textures. these can be constructed directly from a SDL
// surface or loaded from disk
class Texture : public Graphics::Texture {
public:
	Texture(SDL_Surface *s);
	Texture(const std::string &filename);
};

}

#endif
