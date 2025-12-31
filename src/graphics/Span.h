// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <initializer_list>
#include <cstddef>
#include <type_traits>

namespace Graphics {

	/**
	 * Simple pre-C++20 span implementation.
	 *
	 * To enable use with std::initializer_list, you must declare T as const.
	 * If T is a pointer type to a mutable object, declare T as MyType *const
	 * to allow MyType to be mutable through the pointer.
	 */
	template<typename T>
	struct Span {
		Span() {}
		template<typename T1 = T, typename = std::enable_if_t<std::is_const_v<T1>>>
		Span(std::initializer_list<T> list) : m_data(&*list.begin()), m_size(list.size()) {}
		Span(T * data, size_t size) : m_data(data), m_size(size) {};

		bool empty() const { return m_size == 0; }
		size_t size() const { return m_size; }

		T *data() const { return m_data; }

		T* begin() const { return m_data; }
		T* end() const { return m_data + m_size; }

		T &operator[](size_t idx) const { return m_data[idx]; }

	private:
		T* m_data = nullptr;
		size_t m_size = 0;
	};

} // namespace Graphics
