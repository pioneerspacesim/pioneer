// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREDUMMY_H
#define _TEXTUREDUMMY_H

#include "graphics/Texture.h"

namespace Graphics {

	class TextureDummy : public Texture {
	public:
		virtual void Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) override {}
		virtual void Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips) override {}

		void Bind() override {}
		void Unbind() override {}

		virtual void SetSampleMode(TextureSampleMode) override {}
		virtual void BuildMipmaps() override {}
		virtual uint32_t GetTextureID() const override final { return 0U; }

	private:
		friend class RendererDummy;
		TextureDummy(const TextureDescriptor &descriptor) :
			Texture(descriptor) {}
	};

} // namespace Graphics

#endif
