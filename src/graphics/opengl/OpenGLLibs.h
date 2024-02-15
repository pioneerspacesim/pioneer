// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_OPENGLIBS_H
#define _OGL_OPENGLIBS_H

#include <GL/glew.h>

inline const char *glstr_to_str(const GLubyte *str) { return reinterpret_cast<const char *>(str); }

#endif
