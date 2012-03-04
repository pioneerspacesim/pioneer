#ifndef _GUITEXTURE_H
#define _GUITEXTURE_H

#include "graphics/Texture.h"
#include "graphics/Drawables.h"
#include "Color.h"

namespace Graphics { class Renderer; }

namespace Gui {

// subclass for UI textures. these can be constructed directly from a SDL
// surface or loaded from disk
class Texture : public Graphics::Texture {
public:
	Texture(Graphics::Renderer *r, SDL_Surface *s);
	Texture(Graphics::Renderer *r, const std::string &filename);
};


// a textured quad with reversed winding for the UI
class TexturedQuad : public Graphics::Drawables::Drawable {
public:
	TexturedQuad(Graphics::Texture *texture) : m_texture(RefCountedPtr<Graphics::Texture>(texture)) {}
	virtual void Draw(Graphics::Renderer *r) { Draw(r, vector2f(0.0f), vector2f(1.0f)); }
	void Draw(Graphics::Renderer *r, const Color &tint) { Draw(r, vector2f(0.0f), vector2f(1.0f), tint); }
	void Draw(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const Color &tint = Color::WHITE) { Draw(r, pos, size, vector2f(0.0f), vector2f(m_texture->GetTextureWidth(), m_texture->GetTextureHeight()), tint); }
	void Draw(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint = Color::WHITE);
private:
	RefCountedPtr<Graphics::Texture> m_texture;
};

}

#endif
