// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "graphics/opengl/VertexBufferGL.h"

namespace Graphics {

	namespace OGL {
		class UniformLinearBuffer;

		template <typename T>
		struct ScopedMapping {
			ScopedMapping(void *data, Mappable *map) :
				m_data(data), m_map(map)
			{}
			~ScopedMapping()
			{
				if (m_data)
					m_map->Unmap();
			}

			T *operator->() { return m_data; }

		protected:
			T *m_data;
			Mappable *m_map;
		};

		/*
			Implements a linear-allocator style uniform buffer binding.
			Call Allocate to reserve and map a subrange of the buffer for code to write into once.
			All allocations are reset at the start of the next frame.
		*/
		class UniformLinearBuffer : public Mappable, GLBufferBase {
		public:
			UniformLinearBuffer(uint32_t maxSize);
			~UniformLinearBuffer();

			// Don't copy a buffer
			UniformLinearBuffer(const UniformLinearBuffer &) = delete;
			UniformLinearBuffer &operator=(const UniformLinearBuffer &) = delete;

			uint32_t FreeSize() const { return m_capacity - m_size; }
			uint32_t NumAllocs() const { return m_numAllocs; }
			void Reset();
			void Unmap() override;

			template <typename T>
			ScopedMapping<T> Allocate(uint32_t binding)
			{
				assert(m_mapMode == BUFFER_MAP_NONE);
				return ScopedMapping<T>(AllocInternal(binding, sizeof(T)), this);
			}

		protected:
			void *AllocInternal(uint32_t binding, size_t size);
			uint32_t m_numAllocs;
		};

	} // namespace OGL

} // namespace Graphics
