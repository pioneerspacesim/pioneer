#include "libs.h"
#include "PiLuaAPI.h"
#include "Pi.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "Sound.h"
#include "LuaChatForm.h"

////////////////////////////////////////////////////////////

EXPORT_OOLUA_FUNCTIONS_2_NON_CONST(ObjectWrapper,
		SpaceStationAddAdvert,
		SpaceStationRemoveAdvert)
EXPORT_OOLUA_FUNCTIONS_3_CONST(ObjectWrapper,
		print,
		IsBody,
		GetLabel)

ObjectWrapper::ObjectWrapper(Object *o): m_obj(o) {
	m_delCon = o->onDelete.connect(sigc::mem_fun(this, &ObjectWrapper::OnDelete));
}
bool ObjectWrapper::IsBody() const {
	return Is(Object::BODY);
}
const char *ObjectWrapper::GetLabel() const {
	if (Is(Object::BODY)) {
		return static_cast<Body*>(m_obj)->GetLabel().c_str();
	} else {
		return "";
	}
}
void ObjectWrapper::SpaceStationAddAdvert(const char *luaMod, int luaRef, const char *description) {
	if (Is(Object::SPACESTATION)) {
		static_cast<SpaceStation*>(m_obj)->BBAddAdvert(BBAdvert(luaMod, luaRef, description));
	}
}
void ObjectWrapper::SpaceStationRemoveAdvert(const char *luaMod, int luaRef) {
	if (Is(Object::SPACESTATION)) {
		static_cast<SpaceStation*>(m_obj)->BBRemoveAdvert(luaMod, luaRef);
	}
}
int ObjectWrapper::print() const { printf("ObjectWrapper = %p;\n", m_obj); return 1; }

ObjectWrapper::~ObjectWrapper() {
//	printf("ObjWrapper for %s is being deleted\n", GetLabel());
	m_delCon.disconnect();
}
bool ObjectWrapper::Is(Object::Type t) const {
	return m_obj && m_obj->IsType(t);
}
void ObjectWrapper::OnDelete() {
	// object got deleted out from under us
	m_obj = 0;
	m_delCon.disconnect();
}

/////////////////////////////////////////////////////////////

// oolua doesn't like namespaces
class SoundEvent: public Sound::Event {};

OOLUA_CLASS_NO_BASES(SoundEvent)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
	OOLUA_MEM_FUNC_4(void, Play, const char *, float, float, Uint32)
	OOLUA_MEM_FUNC_0(bool, Stop)
OOLUA_CLASS_END

EXPORT_OOLUA_FUNCTIONS_2_NON_CONST(SoundEvent,
		Play, Stop)
EXPORT_OOLUA_FUNCTIONS_0_CONST(SoundEvent)

///////////////////////////////////////////////////////////////

namespace LuaPi {
	static int GetPlayer(lua_State *l) {
		push2luaWithGc(l, new ObjectWrapper((Object*)Pi::player));
		return 1;
	}
	static int GetGameTime(lua_State *l) {
		OOLUA_C_FUNCTION_0(double, Pi::GetGameTime)
	}
}

#define REG_FUNC(fnname, fnptr) \
	lua_pushcfunction(l, fnptr);\
	lua_setfield(l, -2, fnname)

void RegisterPiLuaAPI(lua_State *l)
{
	OOLUA::register_class<ObjectWrapper>(l);
	OOLUA::register_class<LuaChatForm>(l);
	OOLUA::register_class<SoundEvent>(l);

	lua_newtable(l);
	REG_FUNC("GetPlayer", &LuaPi::GetPlayer);
	REG_FUNC("GetGameTime", &LuaPi::GetGameTime);
	lua_setglobal(l, "Pi");
}
