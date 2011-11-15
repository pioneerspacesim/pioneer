#ifndef _SMARTPTR_H
#define _SMARTPTR_H

#include <cstddef>
#include <cassert>

#ifdef __GNUC__
#define WARN_UNUSED_RESULT(ret,decl) ret decl __attribute__ ((warn_unused_result))
#else
#define WARN_UNUSED_RESULT(ret,decl) ret decl
#endif

template <typename Derived, typename T>
class SmartPtrBase {
	typedef SmartPtrBase<Derived, T> this_type;
	typedef T* this_type::*safe_bool;

public:
	// copy & swap idiom
	// see http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
	void Reset(T *p = 0) { Derived(p).Swap(*static_cast<Derived*>(this)); }

	T &operator*() const { assert(m_ptr); return *m_ptr; }
	T *operator->() const { assert(m_ptr); return m_ptr; }
	T *Get() const { return m_ptr; }
	bool Valid() const { return (m_ptr != 0); }
	// safe bool idiom; see http://www.artima.com/cppsource/safebool.html
	operator safe_bool() const { return (m_ptr == 0) ? 0 : &this_type::m_ptr; }
	bool operator!() const { return (m_ptr == 0); }

	friend void swap(this_type &a, this_type &b) { a.Swap(b); }

	void Swap(this_type &b) {
		T *p = m_ptr;
		m_ptr = b.m_ptr;
		b.m_ptr = p;
	}

protected:
	SmartPtrBase(): m_ptr(0) {}
	explicit SmartPtrBase(T *p): m_ptr(p) {}

	// Release() doesn't make sense for all smart pointer types
	// (e.g., RefCountedPtr can't Release, it can only Reset)
	WARN_UNUSED_RESULT(T*,Release()) { T *p = m_ptr; m_ptr = 0; return p; }

	T *m_ptr;

private:
	// disable comparison operators (which are implicitly provided by the conversion to safe_bool)
	bool operator==(const this_type& b);
	bool operator!=(const this_type& b);
};

template <typename T>
class ScopedPtr : public SmartPtrBase<ScopedPtr<T>, T> {
	typedef ScopedPtr<T> this_type;
	typedef SmartPtrBase<this_type, T> base_type;
public:
	ScopedPtr() {}
	explicit ScopedPtr(T *p): base_type(p) {}
	~ScopedPtr() { delete this->Release(); }

	using base_type::Release;
};

template <typename T>
class ScopedArray : public SmartPtrBase<ScopedArray<T>, T> {
	typedef ScopedArray<T> this_type;
	typedef SmartPtrBase<this_type, T> base_type;
public:
	ScopedArray() {}
	explicit ScopedArray(T *p): base_type(p) {}
	~ScopedArray() { delete[] this->Release(); }

	T &operator[](std::ptrdiff_t i) const { return this->m_ptr[i]; }

	using base_type::Release;
};

template <typename T>
class ScopedMalloc : public SmartPtrBase<ScopedMalloc<T>, T> {
	typedef ScopedMalloc<T> this_type;
	typedef SmartPtrBase<this_type, T> base_type;
public:
	ScopedMalloc() {}
	explicit ScopedMalloc(T *p): base_type(p) {}
	// malloc() and co return void pointers, which we kindly convert without question
	explicit ScopedMalloc(void *p): base_type(static_cast<T*>(p)) {}
	~ScopedMalloc() { free(this->Release()); }

	void Reset(void *p) { base_type::Reset(static_cast<T*>(p)); }

	T &operator[](std::ptrdiff_t i) const { return this->m_ptr[i]; }

	using base_type::Release;
};

#endif
