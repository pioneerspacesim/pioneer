// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SDLWrappers.h"
#include "FileSystem.h"
#include "utils.h"

#include <SDL_image.h>

SDLSurfacePtr LoadSurfaceFromFile(const std::string &fname, FileSystem::FileSource &source)
{
	PROFILE_SCOPED()
	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(fname);
	if (!filedata) {
		Output("LoadSurfaceFromFile: %s: could not read file\n", fname.c_str());
		return SDLSurfacePtr();
	}

	SDL_RWops *datastream = SDL_RWFromConstMem(filedata->GetData(), filedata->GetSize());
	SDL_Surface *surface = IMG_Load_RW(datastream, 1);
	if (!surface) {
		Output("LoadSurfaceFromFile: %s: %s\n", fname.c_str(), IMG_GetError());
		return SDLSurfacePtr();
	}

	return SDLSurfacePtr::WrapNew(surface);
}

SDLSurfacePtr LoadSurfaceFromFile(const std::string &fname)
{
	return LoadSurfaceFromFile(fname, FileSystem::gameDataFiles);
}
