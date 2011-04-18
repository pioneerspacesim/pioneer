#include "LuaStar.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Star>::s_type = "Star";

template <> void LuaObject<Star>::RegisterClass() {
	const char *l_inherit = "Body";

	LuaObjectBase::CreateClass(s_type, l_inherit, NULL, NULL);
}
