#include "libs.h"
#include "PiLuaAPI.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "Sound.h"
#include "LuaChatForm.h"
#include "NameGenerator.h"

////////////////////////////////////////////////////////////

EXPORT_OOLUA_FUNCTIONS_3_NON_CONST(ObjectWrapper,
		SetMoney,
		SpaceStationAddAdvert,
		SpaceStationRemoveAdvert)
EXPORT_OOLUA_FUNCTIONS_4_CONST(ObjectWrapper,
		print,
		IsBody,
		GetMoney,
		GetLabel)

ObjectWrapper::ObjectWrapper(Object *o): m_obj(o) {
	m_delCon = o->onDelete.connect(sigc::mem_fun(this, &ObjectWrapper::OnDelete));
}
bool ObjectWrapper::IsBody() const {
	return Is(Object::BODY);
}
double ObjectWrapper::GetMoney() const {
	if (Is(Object::SHIP)) {
		Ship *s = static_cast<Ship*>(m_obj);
		return 0.01 * s->GetMoney();
	} else {
		return 0;
	}
}

void ObjectWrapper::SetMoney(double m) {
	if (Is(Object::SHIP)) {
		Ship *s = static_cast<Ship*>(m_obj);
		s->SetMoney((Sint64)(m*100.0));
	}
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
	static int _RandInt(int min, int max) { return Pi::rng.Int32(min, max); }
	static int RandInt(lua_State *l) {
		OOLUA_C_FUNCTION_2(int, _RandInt, int, int);
	}
	static double _RandReal(double min, double max) { return Pi::rng.Double(min, max); }
	static int RandReal(lua_State *l) {
		OOLUA_C_FUNCTION_2(double, _RandReal, double, double);
	}
	static int Message(lua_State *l) {
		std::string from, msg;
		OOLUA::pull2cpp(l, msg);
		OOLUA::pull2cpp(l, from);
		Pi::cpan->MsgLog()->Message(from, msg);
		return 0;
	}
	static int RandPersonName(lua_State *l) {
		bool genderFemale;
		OOLUA::pull2cpp(l, genderFemale);
		std::string name = NameGenerator::FullName(Pi::rng, genderFemale);
		OOLUA::push2lua(l, name.c_str());
		return 1;
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
	REG_FUNC("Message", &LuaPi::Message);
	lua_setglobal(l, "Pi");
	
	lua_newtable(l);
	REG_FUNC("Int", &LuaPi::RandInt);
	REG_FUNC("Real", &LuaPi::RandReal);
	REG_FUNC("PersonName", &LuaPi::RandPersonName);
	lua_setglobal(l, "Rand");
}
