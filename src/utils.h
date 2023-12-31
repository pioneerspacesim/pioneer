// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _UTILS_H
#define _UTILS_H

#include "core/Log.h"
#include "core/StringUtils.h"
#include "core/macros.h"

#include "profiler/Profiler.h"

// An adaptor for automagic reverse range-for iteration of containers
// One might be able to specialize this for raw arrays, but that's beyond the
// point of its use-case.
// One might also point out that this is surely more work to code than simply
// writing an explicit iterator loop, to which I say: bah humbug!
template <typename T>
struct reverse_container_t {
	using iterator = typename T::reverse_iterator;
	using const_iterator = typename T::const_reverse_iterator;

	using value_type = typename std::remove_reference<T>::type;

	reverse_container_t(value_type &ref) :
		ref(ref) {}

	iterator begin() { return ref.rbegin(); }
	const_iterator begin() const { return ref.crbegin(); }

	iterator end() { return ref.rend(); }
	const_iterator end() const { return ref.crend(); }

private:
	value_type &ref;
};

// Use this function for automatic template parameter deduction
template <typename T>
reverse_container_t<T> reverse_container(T &ref) { return reverse_container_t<T>(ref); }

#endif /* _UTILS_H */
