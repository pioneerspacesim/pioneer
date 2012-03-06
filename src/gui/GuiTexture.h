#ifndef _GUITEXTURE_H
#define _GUITEXTURE_H

#include "graphics/TextureDescriptor.h"
#include "graphics/Texture.h"
#include "graphics/Drawables.h"
#include "Color.h"

namespace Graphics { class Renderer; }

namespace Gui {

class SurfaceTextureDescriptor : public Graphics::TextureDescriptor {
public:
	SurfaceTextureDescriptor(const std::string &name, SDL_Surface *surface = 0);

	virtual const Graphics::TextureDescriptor::Data *GetData() const;

	virtual bool IsEqual(const TextureDescriptor &b) const {
		if (!TextureDescriptor::IsEqual(b)) return false;
		const SurfaceTextureDescriptor *bb = dynamic_cast<const SurfaceTextureDescriptor*>(&b);
		return (bb && bb->name == name);
	}

	virtual SurfaceTextureDescriptor *Clone() const {
		return new SurfaceTextureDescriptor(*this);
	}

	const std::string name;

private:
	SDL_Surface *m_surface;
};


class FileTextureDescriptor : public Graphics::TextureDescriptor {
public:
	FileTextureDescriptor(const std::string &filename);

	virtual const Graphics::TextureDescriptor::Data *GetData() const;

	virtual bool IsEqual(const TextureDescriptor &b) const {
		if (!TextureDescriptor::IsEqual(b)) return false;
		const FileTextureDescriptor *bb = dynamic_cast<const FileTextureDescriptor*>(&b);
		return (bb && bb->filename == filename);
	}

	virtual FileTextureDescriptor *Clone() const {
		return new FileTextureDescriptor(*this);
	}

	const std::string filename;
};


// a textured quad with reversed winding for the UI
class TexturedQuad : public Graphics::Drawables::Drawable {
public:
	TexturedQuad(Graphics::Texture *texture) : m_texture(texture) {}
	virtual void Draw(Graphics::Renderer *r) { Draw(r, vector2f(0.0f), vector2f(1.0f)); }
	void Draw(Graphics::Renderer *r, const Color &tint) { Draw(r, vector2f(0.0f), vector2f(1.0f), tint); }
	void Draw(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const Color &tint = Color::WHITE) { Draw(r, pos, size, vector2f(0.0f), m_texture->GetTextureSize(), tint); }
	void Draw(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint = Color::WHITE);
private:
	Graphics::Texture *m_texture;
};

}

#endif
