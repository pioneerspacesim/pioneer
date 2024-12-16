// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREGL_H
#define _TEXTUREGL_H

#include "OpenGLLibs.h"
#include "graphics/Texture.h"

namespace Graphics {
	namespace OGL {
		class TextureGL : public Texture {
		public:
			void Update(const void *data, const vector2f &pos, const vector3f &dataSize, TextureFormat format, const unsigned int numMips) final;
			void Update(const TextureCubeData &data, const vector3f &dataSize, TextureFormat format, const unsigned int numMips) final;
			void Update(const vecDataPtr &data, const vector3f &dataSize, const TextureFormat format, const unsigned int numMips) final;

			TextureGL(const TextureDescriptor &descriptor, const bool useCompressed, const bool useAnisoFiltering, const Uint16 numSamples = 0);
			virtual ~TextureGL();

			void Bind() final;
			void Unbind() final;

			void SetSampleMode(TextureSampleMode) final;
			void BuildMipmaps(const uint32_t validMips = 1) final;
			uint32_t GetTextureID() const final
			{
				static_assert(sizeof(uint32_t) == sizeof(GLuint));
				return m_texture;
			}

			uint32_t GetTarget() const
			{
				static_assert(sizeof(uint32_t) == sizeof(GLuint));
				return m_target;
			}

			uint32_t GetTextureMemSize() const final { return m_allocSize; }

		private:
			GLenum m_target;
			GLuint m_texture;
			uint32_t m_allocSize;
			const bool m_useAnisoFiltering;
		};
	} // namespace OGL
} // namespace Graphics

#endif
