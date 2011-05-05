#include "LuaStar.h"
#include "LuaUtils.h"

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<Star*>(o);
}

template <> const char *LuaObject<Star>::s_type = "Star";

template <> void LuaObject<Star>::RegisterClass() {
	const char *l_parent = "Body";

	LuaObjectBase::CreateClass(s_type, l_parent, NULL, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
