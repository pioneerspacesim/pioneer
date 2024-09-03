// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <SDL_stdinc.h>
#include <string>

namespace Graphics {
	struct ScreendumpState;
}

void write_screenshot(const Graphics::ScreendumpState &sd, const char *destFile);

#endif
