// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BUILDOPTS_H
#define _BUILDOPTS_H

// game version. usually defined by configure
#ifndef PIONEER_VERSION
#define PIONEER_VERSION "unknown"
#endif
#ifndef PIONEER_EXTRAVERSION
#define PIONEER_EXTRAVERSION ""
#endif

// define to include the object viewer in the build
#ifndef WITH_OBJECTVIEWER
#define WITH_OBJECTVIEWER 1
#endif

// define to include various extra keybindings for dev functions
#ifndef WITH_DEVKEYS
#define WITH_DEVKEYS 1
#endif

#ifdef _MSC_VER // MSVC doesn't support the %z specifier, but has its own %I specifier
#define SIZET_FMT "%Iu"
#else
#define SIZET_FMT "%zu"
#endif

#endif
