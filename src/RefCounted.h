// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _REFCOUNTED_H
#define _REFCOUNTED_H

#include "SmartPtr.h"
#include "lua/LuaWrappable.h"
#include <atomic>

class RefCounted : public LuaWrappable {
public:
	RefCounted() :
		m_refCount(0) {}
	virtual ~RefCounted() {}

	inline void IncRefCount() const { ++m_refCount; }
	inline void DecRefCount() const
	{
		assert(m_refCount > 0);
		if (!--m_refCount) delete this;
	}
	inline int GetRefCount() const { return m_refCount; }

private:
	// vs2012 doesn't support the `= delete` syntax
	RefCounted(const RefCounted &);
	RefCounted(const RefCounted &&);
	RefCounted &operator=(const RefCounted &);
	RefCounted &operator=(const RefCounted &&);

	mutable std::atomic<int> m_refCount; // We model logical constness, so refcount is changeable even for const objects
};

template <typename T>
class RefCountedPtr : public SmartPtrBase<RefCountedPtr<T>, T> {
	typedef RefCountedPtr<T> this_type;
	typedef SmartPtrBase<this_type, T> base_type;

public:
	RefCountedPtr() {}

	explicit RefCountedPtr(T *p) :
		base_type(p)
	{
		if (this->m_ptr) this->m_ptr->IncRefCount();
	}

	// Note: This is still needed, the compiler would generate a default copy constructor despite the templated version
	RefCountedPtr(const this_type &b) :
		base_type(b.Get())
	{
		if (this->m_ptr) this->m_ptr->IncRefCount();
	}

	// Generalized copy constructor, allowing copy for compatible pointer types (e.g. RefCountedPtr<const T>)
	template <typename U>
	RefCountedPtr(const RefCountedPtr<U> &b) :
		base_type(b.Get())
	{
		if (this->m_ptr) this->m_ptr->IncRefCount();
	}

	~RefCountedPtr()
	{
		T *p = this->Release();
		if (p) p->DecRefCount();
	}

	// Note: This is still needed, the compiler would generate a default assignment operator despite the templated version
	this_type &operator=(const this_type &b)
	{
		this->Reset(b.Get());
		return *this;
	}

	// Generalized assignment operator allowing assignment for compatible pointer types (e.g. RefCountedPtr<const T>)
	template <typename U>
	this_type &operator=(const RefCountedPtr<U> &b)
	{
		this->Reset(b.Get());
		return *this;
	}

	bool Unique() const
	{
		assert(this->m_ptr);
		return (this->m_ptr->GetRefCount() == 1);
	}
};

#endif
