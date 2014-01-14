// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _REFCOUNTED_H
#define _REFCOUNTED_H

#include "SmartPtr.h"
#include "LuaWrappable.h"

class RefCounted : public LuaWrappable {
public:
	RefCounted() : m_refCount(0) {}
	virtual ~RefCounted() {}

	inline void IncRefCount() { m_refCount++; }
	inline void DecRefCount() { assert(m_refCount > 0); if (! --m_refCount) delete this; }
	inline int GetRefCount() const { return m_refCount; }

private:
	int m_refCount;
};

template <typename T>
class RefCountedPtr : public SmartPtrBase<RefCountedPtr<T>, T> {
	typedef RefCountedPtr<T> this_type;
	typedef SmartPtrBase<this_type, T> base_type;
public:
	RefCountedPtr() {}

	explicit RefCountedPtr(T *p): base_type(p)
	{ if (this->m_ptr) this->m_ptr->IncRefCount(); }

	RefCountedPtr(const this_type& b): base_type(b.Get())
	{ if (this->m_ptr) this->m_ptr->IncRefCount(); }

	~RefCountedPtr() {
		T *p = this->Release();
		if (p) p->DecRefCount();
	}

	this_type &operator=(const this_type &b) { this->Reset(b.Get()); return *this; }

	bool Unique() const { assert(this->m_ptr); return (this->m_ptr->GetRefCount() == 1); }
};

#endif
