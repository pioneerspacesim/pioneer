// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"
#include "graphics/Types.h"

/*
 * This file contains code common to the implementation of different GPU data
 * buffer classes. It provides a common interface for sending data to buffers
 * and binding those buffers to the renderer interface.
 */
namespace Graphics {

	class Mappable : public RefCounted {
	public:
		virtual ~Mappable() {}
		virtual void Unmap() = 0;

		inline uint32_t GetSize() const { return m_size; }
		inline uint32_t GetCapacity() const { return m_capacity; }

	protected:
		explicit Mappable(const uint32_t size) :
			m_mapMode(BUFFER_MAP_NONE),
			m_size(size),
			m_capacity(size) {}
		BufferMapMode m_mapMode; //tracking map state

		// size is the current number of elements in the buffer
		uint32_t m_size;
		// capacity is the maximum number of elements that can be put in the buffer
		uint32_t m_capacity;
	};

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

	template <typename T>
	struct BufferBinding {
		T *buffer;
		uint32_t offset;
		uint32_t size;

		bool operator!=(const BufferBinding &rhs) const { return !(*this == rhs); }
		bool operator==(const BufferBinding &rhs) const
		{
			return buffer == rhs.buffer && offset == rhs.offset && size == rhs.size;
		}
	};

} // namespace Graphics
