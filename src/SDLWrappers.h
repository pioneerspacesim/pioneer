#ifndef _SDLWRAPPERS_H
#define _SDLWRAPPERS_H

#include "SmartPtr.h"

struct SDL_Surface;

class SDLSurfacePtr : public SmartPtrBase<SDLSurfacePtr, SDL_Surface> {
	typedef SmartPtrBase<SDLSurfacePtr, SDL_Surface> base_type;
public:
	SDLSurfacePtr() {}
	explicit SDLSurfacePtr(SDL_Surface *p): base_type(p) {}
	SDLSurfacePtr(const SDLSurfacePtr& b): base_type(b.Get())
	{ if (this->m_ptr) this->m_ptr->refcount += 1; }
	~SDLSurfacePtr() { SDL_FreeSurface(this->Release()); }

	SDLSurfacePtr &operator=(const SDLSurfacePtr &b) { SDLSurfacePtr(b).Swap(*this); return *this; }

	bool Unique() const { assert(this->m_ptr); return (this->m_ptr->refcount == 1); }
};

#endif
