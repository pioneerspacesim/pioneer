// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "versioningInfo.h"
#include "libs.h"
#include "utils.h"
// open asset importer
#include <assimp/version.h>
// freetype
#include "ft2build.h"
#include FT_FREETYPE_H

#ifdef ENABLE_SERVER_AGENT
#include <curl/curlver.h>
#endif

void OutputVersioningInfo()
{
	SDL_version ver;
	SDL_GetVersion(&ver);
	Output("\n--------------------\n");
	Output("SDL Version %d.%d.%d\n", ver.major, ver.minor, ver.patch);
	Output("SDL_image ver: %d.%d.%d\n", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
	Output("Assimp ver: %u.%u.%u\n", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	Output("FreeType ver: %d.%d.%d\n", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);

#ifdef ENABLE_SERVER_AGENT
	Output("LibCurl ver: %s", LIBCURL_VERSION);
#endif
	Output("\n--------------------\n");
}
