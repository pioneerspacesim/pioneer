#include "LuaStar.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Star>::s_type = "Star";

template <> void LuaObject<Star>::RegisterClass() {
	LuaObjectBase::CreateClass(s_type, NULL, NULL, NULL);
}
