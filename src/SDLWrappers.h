// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SDLWRAPPERS_H
#define _SDLWRAPPERS_H

#include "SmartPtr.h"
#include <SDL_surface.h>
#include <string>

namespace FileSystem {
	class FileSource;
}

struct SDL_Surface;

class SDLSurfacePtr : public SmartPtrBase<SDLSurfacePtr, SDL_Surface> {
	typedef SmartPtrBase<SDLSurfacePtr, SDL_Surface> base_type;

private:
	explicit SDLSurfacePtr(SDL_Surface *p) :
		base_type(p) {}

public:
	static SDLSurfacePtr WrapNew(SDL_Surface *p) { return SDLSurfacePtr(p); }
	static SDLSurfacePtr WrapCopy(SDL_Surface *p)
	{
		if (p) {
			p->refcount += 1;
		}
		return SDLSurfacePtr(p);
	}

	SDLSurfacePtr() {}
	SDLSurfacePtr(const SDLSurfacePtr &b) :
		base_type(b.Get())
	{
		if (this->m_ptr) this->m_ptr->refcount += 1;
	}
	//	~SDLSurfacePtr() { SDL_FreeSurface(this->Release()); }
	// workaround for SDL 2.0.6 FreeSurface bug
	~SDLSurfacePtr()
	{
		SDL_Surface *p = this->Release();
		if (p && --p->refcount <= 0) SDL_FreeSurface(p);
	}

	SDLSurfacePtr &operator=(const SDLSurfacePtr &b)
	{
		SDLSurfacePtr(b).Swap(*this);
		return *this;
	}

	bool Unique() const
	{
		assert(this->m_ptr);
		return (this->m_ptr->refcount == 1);
	}
};

SDLSurfacePtr LoadSurfaceFromFile(const std::string &fname, FileSystem::FileSource &source);
SDLSurfacePtr LoadSurfaceFromFile(const std::string &fname);

#endif
