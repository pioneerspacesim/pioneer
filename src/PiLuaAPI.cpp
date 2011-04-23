#include "libs.h"
#include "PiLuaAPI.h"
#include "Pi.h"
#include "Space.h"
#include "ShipCpanel.h"
#include "Ship.h"
#include "Player.h"
#include "SpaceStation.h"
#include "StarSystem.h"
#include "Sound.h"
#include "LuaChatForm.h"
#include "NameGenerator.h"
#include "HyperspaceCloud.h"
#include "Polit.h"
#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaPlayer.h"

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


void RegisterPiLuaAPI(lua_State *l)
{
	LUA_DEBUG_START(l);
	OOLUA::register_class<SoundEvent>(l);
	LUA_DEBUG_END(l, 0);
}
