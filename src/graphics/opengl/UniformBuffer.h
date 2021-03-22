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
				m_data(reinterpret_cast<T *>(data)), m_map(map)
			{}
			~ScopedMapping()
			{
				if (m_data)
					m_map->Unmap();
			}

			// don't allow copying a scoped mapping
			ScopedMapping(const ScopedMapping &) = delete;
			ScopedMapping &operator=(const ScopedMapping) = delete;

			T *operator->() { return m_data; }
			T *data() { return m_data; }
			operator bool() const { return isValid(); }
			bool isValid() const { return m_data != nullptr; }

		protected:
			T *m_data;
			Mappable *m_map;
		};

		class UniformBuffer;

		struct UniformBufferBinding {
			UniformBuffer *buffer;
			uint32_t offset;
			uint32_t size;
		};

		class UniformBuffer : public Mappable, public GLBufferBase {
		public:
			UniformBuffer(uint32_t size, BufferUsage usage);
			~UniformBuffer();

			void Unmap() override;

			template <typename T>
			ScopedMapping<T> Map(BufferMapMode mode)
			{
				return ScopedMapping<T>(reinterpret_cast<T *>(MapInternal(mode)), this);
			}

			// copies the contents of the passed object into the buffer
			template <typename T>
			void BufferData(const T &obj)
			{
				return BufferData(sizeof(T), (void *)&obj);
			}

			// change the buffer data without mapping
			void BufferData(const size_t, void *);

			void Bind(uint32_t binding);
			void BindRange(uint32_t binding, uint32_t offset, uint32_t range);
			void Release();

		protected:
			void *MapInternal(BufferMapMode mode);
		};

		/*
			Implements a linear-allocator style uniform buffer binding.
			Call Allocate to reserve and map a subrange of the buffer for code to write into once.
			All allocations are reset at the start of the next frame.
		*/
		class UniformLinearBuffer : public UniformBuffer {
		public:
			UniformLinearBuffer(uint32_t maxSize);
			~UniformLinearBuffer();

			// Don't copy a buffer
			UniformLinearBuffer(const UniformLinearBuffer &) = delete;
			UniformLinearBuffer &operator=(const UniformLinearBuffer &) = delete;

			// public for ScopedMapping
			void Unmap() override;

			// Flushes all written data so far to the uniform buffer.
			// Call this once before executing a command list to ensure data
			// is made visible to the GPU
			void Flush();

			// Resets all allocations to the buffer and orphans the previous data,
			// making it ready for a new frame.
			void Reset();

			uint32_t FreeSize() const { return m_capacity - m_size; }
			uint32_t NumAllocs() const { return m_numAllocs; }

			template <typename T>
			ScopedMapping<T> Allocate(UniformBufferBinding &outBinding)
			{
				assert(m_mapMode == BUFFER_MAP_NONE);
				return ScopedMapping<T>(AllocInternal(sizeof(T), outBinding), this);
			}

			UniformBufferBinding Allocate(void *data, size_t size);

		protected:
			// protect some methods we don't want to allow public access to
			using UniformBuffer::BufferData;
			using UniformBuffer::Map;

			void *AllocInternal(size_t size, UniformBufferBinding &outBinding);
			// cache individual allocations into a single buffer and upload to
			// the GPU in one large chunk.
			std::unique_ptr<char[]> m_data;

			// This tracks the end of the last section of flushed data so
			// we can use the same allocator with multiple command lists
			uint32_t m_lastFlush;
			uint32_t m_numAllocs;
		};

	} // namespace OGL

} // namespace Graphics
