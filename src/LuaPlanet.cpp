#include "LuaPlanet.h"
#include "LuaUtils.h"

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<Planet*>(o);
}

template <> const char *LuaObject<Planet>::s_type = "Planet";

template <> void LuaObject<Planet>::RegisterClass()
{
	const char *l_inherit = "Body";

	LuaObjectBase::CreateClass(s_type, l_inherit, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_inherit, s_type, promotion_test);
}
