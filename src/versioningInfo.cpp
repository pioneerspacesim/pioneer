// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "versioningInfo.h"

#include "utils.h"
// open asset importer
#include <assimp/version.h>
// freetype
#include "ft2build.h"
#include FT_FREETYPE_H

#ifdef ENABLE_SERVER_AGENT
#include <curl/curlver.h>
#endif

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>

void OutputVersioningInfo()
{
	SDL_version ver;
	SDL_GetVersion(&ver);
	Output("--------------------\n");
	Output("SDL Version (build) %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
	Output("SDL Version (dynamic) %d.%d.%d\n", ver.major, ver.minor, ver.patch);
	const bool sameSDLVer = (SDL_MAJOR_VERSION == ver.major && SDL_MINOR_VERSION == ver.minor && SDL_PATCHLEVEL == ver.patch);
	Output(sameSDLVer ? "SDL Versions match\n" : "WARNING: SDL Versions differ\n");
	Output("SDL_image Version (build): %d.%d.%d\n", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
	const SDL_version *pVer = IMG_Linked_Version();
	Output("SDL_image Version (dynamic): %d.%d.%d\n", pVer->major, pVer->minor, pVer->patch);
	const bool sameSDLImageVer = (SDL_IMAGE_MAJOR_VERSION == pVer->major && SDL_IMAGE_MINOR_VERSION == pVer->minor && SDL_IMAGE_PATCHLEVEL == pVer->patch);
	Output(sameSDLImageVer ? "SDL_image Versions match\n" : "WARNING: SDL_image Versions differ\n");
	Output("Assimp Version: %u.%u.%u\n", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	Output("FreeType Version: %d.%d.%d\n", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);

#ifdef ENABLE_SERVER_AGENT
	Output("LibCurl Version: %s", LIBCURL_VERSION);
#endif

	// glewGetString returns unsigned, fmt uses signed
	Output("GLEW dynamic version: %s\n", reinterpret_cast<const char *>(glewGetString(GLEW_VERSION)));
	Output("--------------------\n");
	Output("\n");
}
