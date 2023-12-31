// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/BufferCommon.h"
#include "graphics/Types.h"

namespace Graphics {

	class UniformBuffer : public Mappable {
	public:
		UniformBuffer(uint32_t size, BufferUsage usage) :
			Mappable(size),
			m_usage(usage)
		{}
		virtual ~UniformBuffer(){};

		template <typename T>
		ScopedMapping<T> Map(BufferMapMode mode)
		{
			return ScopedMapping<T>(reinterpret_cast<T *>(MapInternal(mode)), this);
		}

		virtual void Unmap() = 0;

		// copies the contents of the passed object into the buffer
		template <typename T>
		void BufferData(const T &obj)
		{
			return BufferData(sizeof(T), (void *)&obj);
		}

		// change the buffer data without mapping
		virtual void BufferData(const size_t, void *) = 0;

		BufferBinding<UniformBuffer> GetBufferBinding() { return { this, 0, m_size }; }

	protected:
		virtual void *MapInternal(BufferMapMode mode) = 0;
		BufferUsage m_usage;
	};

} // namespace Graphics
