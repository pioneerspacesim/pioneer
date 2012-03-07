#ifndef _WORLDTEXTURE_H
#define _WORLDTEXTURE_H

#include "graphics/TextureDescriptor.h"

namespace Graphics { class Renderer; }

class WorldTextureDescriptor : public Graphics::TextureDescriptor {
public:
	WorldTextureDescriptor(const std::string &filename);

	virtual const Graphics::TextureDescriptor::Data *GetData() const;

	virtual bool Compare(const TextureDescriptor &b) const {
		if (type != b.type) return TextureDescriptor::Compare(b);
		const WorldTextureDescriptor &bb = static_cast<const WorldTextureDescriptor&>(b);
        return (filename < bb.filename);
	}

	virtual WorldTextureDescriptor *Clone() const {
		return new WorldTextureDescriptor(*this);
	}

	const std::string filename;
};

#endif
