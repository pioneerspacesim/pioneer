// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#if _DEBUG
#define STRINGTO(x) #x
#define TOSTRING(x) STRINGTO(x)
#define PI_TODO(x) __pragma(message("TODO( " __FILE__ ":" TOSTRING(__LINE__) "):" STRINGTO(x)))
#else
#define PI_TODO(x)
#endif // #if _DEBUG
