#ifndef _REFCOUNTED_H
#define _REFCOUNTED_H

class RefCounted {
public:
	RefCounted() : m_refCount(0) {}
	virtual ~RefCounted() {}

	inline void IncRefCount() { m_refCount++; }
	inline void DecRefCount() { m_refCount--; }
	inline int GetRefCount() { return m_refCount; }

private:
	int m_refCount;
};

#endif
