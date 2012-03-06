#ifndef _WORLDTEXTURE_H
#define _WORLDTEXTURE_H

#include "graphics/TextureDescriptor.h"

namespace Graphics { class Renderer; }

class WorldTextureDescriptor : public Graphics::TextureDescriptor {
public:
	WorldTextureDescriptor(const std::string &filename);

	virtual const Graphics::TextureDescriptor::Data *GetData() const;

	virtual bool IsEqual(const TextureDescriptor &b) const {
		if (!TextureDescriptor::IsEqual(b)) return false;
		const WorldTextureDescriptor *bb = dynamic_cast<const WorldTextureDescriptor*>(&b);
		return (bb && bb->filename == filename);
	}

	virtual WorldTextureDescriptor *Clone() const {
		return new WorldTextureDescriptor(*this);
	}

	const std::string filename;
};

#endif
