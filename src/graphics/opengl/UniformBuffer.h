// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Types.h"
#include "graphics/UniformBuffer.h"
#include "graphics/opengl/GLBufferBase.h"

#include <memory>

namespace Graphics {

	namespace OGL {

		class UniformBuffer : public Graphics::UniformBuffer, public GLBufferBase {
		public:
			UniformBuffer(uint32_t size, BufferUsage usage);
			~UniformBuffer() override;

			void Unmap() override;

			// change the buffer data without mapping
			void BufferData(const size_t, void *) override;

			void BindRange(uint32_t binding, uint32_t offset, uint32_t range);
			void Release();

		protected:
			void *MapInternal(BufferMapMode mode) override;
		};

		/*
			Implements a linear-allocator style uniform buffer binding.
			Call Allocate to reserve and map a subrange of the buffer for code to write into once.
			All allocations are reset at the start of the next frame.
			TODO: there should be a better interface for attaching linear allocators to generic buffers.
		*/
		class UniformLinearBuffer : public UniformBuffer {
		public:
			UniformLinearBuffer(uint32_t maxSize);
			~UniformLinearBuffer() override;

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
			ScopedMapping<T> Allocate(BufferBinding<UniformBuffer> &outBinding)
			{
				assert(m_mapMode == BUFFER_MAP_NONE);
				return ScopedMapping<T>(AllocInternal(sizeof(T), outBinding), this);
			}

			BufferBinding<UniformBuffer> Allocate(void *data, size_t size);

		protected:
			// protect some methods we don't want to allow public access to
			using UniformBuffer::BufferData;
			using UniformBuffer::Map;

			void *AllocInternal(size_t size, BufferBinding<UniformBuffer> &outBinding);

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
