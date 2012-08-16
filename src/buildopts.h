#ifndef _BUILDOPTS_H
#define _BUILDOPTS_H

// game version. usually defined by configure
#ifndef PIONEER_VERSION
#define PIONEER_VERSION "alpha 26 dev"
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
