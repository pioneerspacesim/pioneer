#ifndef _BUILDOPTS_H
#define _BUILDOPTS_H

// game version. usually defined by configure
#ifndef PIONEER_VERSION
#define PIONEER_VERSION "alpha 17 dev"
#endif
#ifndef PIONEER_EXTRAVERSION
#define PIONEER_EXTRAVERSION ""
#endif

// define to include the object viewer in the build
#ifndef OBJECTVIEWER
#define OBJECTVIEWER 1
#endif

// define to include various extra keybindings for dev functions
#ifndef DEVKEYS
#define DEVKEYS 1
#endif

#endif
