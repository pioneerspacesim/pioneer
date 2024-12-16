// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREDUMMY_H
#define _TEXTUREDUMMY_H

#include "graphics/Texture.h"

namespace Graphics {

	class TextureDummy : public Texture {
	public:
		void Update(const void *data, const vector2f &pos, const vector3f &dataSize, TextureFormat format, const unsigned int numMips) final {}
		void Update(const TextureCubeData &data, const vector3f &dataSize, TextureFormat format, const unsigned int numMips) final {}
		void Update(const vecDataPtr &data, const vector3f &dataSize, const TextureFormat format, const unsigned int numMips) final {}

		void Bind() override {}
		void Unbind() override {}

		void SetSampleMode(TextureSampleMode) override {}
		void BuildMipmaps(const uint32_t) override {}
		uint32_t GetTextureID() const final { return 0U; }
		uint32_t GetTextureMemSize() const final { return 0U; }

	private:
		friend class RendererDummy;
		TextureDummy(const TextureDescriptor &descriptor) :
			Texture(descriptor) {}
	};

} // namespace Graphics

#endif
