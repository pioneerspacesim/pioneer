#include <map>
#include <stdio.h>

namespace Lang {
static std::map<const char *,const char **> s_tokens;
}

#define DECLARE_STRING(x)                               \
	const char *x;                                      \
	static class _init_class_##x {                      \
	public:                                             \
		_init_class_##x() {                             \
			s_tokens.insert(std::make_pair(#x, &x));    \
		}                                               \
	} _init_##x

#include "Lang.h"

namespace Lang {

void LoadStrings(std::string lang)
{
	assert(0);
}

}
