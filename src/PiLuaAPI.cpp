#include "libs.h"
#include "PiLuaAPI.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "StarSystem.h"
#include "Sound.h"
#include "LuaChatForm.h"
#include "NameGenerator.h"

////////////////////////////////////////////////////////////

EXPORT_OOLUA_FUNCTIONS_5_NON_CONST(ObjectWrapper,
		SetMoney,
		SpaceStationAddAdvert,
		SpaceStationRemoveAdvert,
		GetDockedWith,
		GetSBody)
EXPORT_OOLUA_FUNCTIONS_3_CONST(ObjectWrapper,
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
SBodyPath *ObjectWrapper::GetSBody()
{
	const SBody *sbody = 0;
	if (Is(Object::BODY)) {
		sbody = static_cast<Body*>(m_obj)->GetSBody();
		if (sbody) {
			SBodyPath *path = new SBodyPath;
			Pi::currentSystem->GetPathOf(sbody, path);
			return path;
		}
	}
	return 0;
}
ObjectWrapper *ObjectWrapper::GetDockedWith()
{
	if (Is(Object::SHIP) && static_cast<Ship*>(m_obj)->GetDockedWith()) {
		return new ObjectWrapper(static_cast<Ship*>(m_obj)->GetDockedWith());
	} else {
		return 0;
	}
}

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

static int UserDataSerialize(lua_State *L)
{
	ObjectWrapper *o;
	luaL_checktype(L, 1, LUA_TUSERDATA);
	if (mylua_checkudata(L, 1, "ObjectWrapper")) {
		OOLUA::pull2cpp(L, o);
		char buf[128];
		// XXX this is a rather hairy cast but should always be true
		assert(static_cast<ObjectWrapper*>(o)->IsBody());
		snprintf(buf, sizeof(buf), "ObjectWrapper\n%d\n", Serializer::LookupBody((Body*)static_cast<ObjectWrapper*>(o)->m_obj));
		lua_pushstring(L, buf);
		return 1;
	} else if (mylua_checkudata(L, 1, "SBodyPath")) {
		Serializer::Writer wr;
		SBodyPath *path;
		OOLUA::pull2cpp(L, path);
		path->Serialize(wr);
		std::string out = "SBodyPath\n";
		out += wr.GetData();
		OOLUA::push2lua(L, out);
		return 1;
	} else if (mylua_checkudata(L, 1, "SysLoc")) {
		Serializer::Writer wr;
		SysLoc *systemid;
		OOLUA::pull2cpp(L, systemid);
		systemid->Serialize(wr);
		std::string out = "SysLoc\n";
		out += wr.GetData();
		OOLUA::push2lua(L, out);
		return 1;
	} else {
		Error("Tried to serialize unknown userdata type.");
		return 0;
	}
}

static int UserDataUnserialize(lua_State *L)
{
	std::string str;
	OOLUA::pull2cpp(L, str);
	if (str.substr(0, 14) == "ObjectWrapper\n") {
		size_t idx = atoi(str.substr(14).c_str());
		Body *b = Serializer::LookupBody(idx);
		push2luaWithGc(L, new ObjectWrapper(b));
		return 1;
	} else if (str.substr(0, 10) == "SBodyPath\n") {
		Serializer::Reader r(str.substr(10));
		SBodyPath *p = new SBodyPath;
		SBodyPath::Unserialize(r, p);
		push2luaWithGc(L, p);
		return 1;
	} else if (str.substr(0, 7) == "SysLoc\n") {
		Serializer::Reader r(str.substr(7));
		SysLoc *p = new SysLoc;
		SysLoc::Unserialize(r, p);
		push2luaWithGc(L, p);
		return 1;
	}
	return 0;
}

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
	static int GetCurrentSystem(lua_State *l) {
		// sadly must rebuild for the mo
		StarSystem *cur = Pi::currentSystem;
		SysLoc *s = new SysLoc(cur->SectorX(), cur->SectorY(), cur->SystemIdx());
		push2luaWithGc(l, s);
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
	OOLUA::register_class<SysLoc>(l);
	OOLUA::register_class<SBodyPath>(l);
	
	lua_register(l, "UserDataSerialize", UserDataSerialize);
	lua_register(l, "UserDataUnserialize", UserDataUnserialize);

	lua_newtable(l);
	REG_FUNC("GetCurrentSystem", &LuaPi::GetCurrentSystem);
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
