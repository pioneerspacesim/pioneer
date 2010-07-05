#include "ShipType.h"
#include "LmrModel.h"
#include "Serializer.h"
#include "MyLuaMathTypes.h"
#include "Pi.h"
#include "utils.h"

const char *ShipType::gunmountNames[GUNMOUNT_MAX] = {
	"Front", "Rear" };

std::map<ShipType::Type, ShipType> ShipType::types;
std::string ShipType::LADYBIRD				= "Ladybird Starfighter";
std::string ShipType::SIRIUS_INTERDICTOR	= "Sirius Interdictor";
std::string ShipType::MISSILE_GUIDED		= "MISSILE_GUIDED";
std::string ShipType::MISSILE_NAVAL			= "MISSILE_NAVAL";
std::string ShipType::MISSILE_SMART			= "MISSILE_SMART";
std::string ShipType::MISSILE_UNGUIDED		= "MISSILE_UNGUIDED";


int ShipType::define_ship(lua_State *L, const char *model_name)
{
	ShipType s;
	s.name = luaPi_gettable_checkstring(L, -1, 1);

	s.lmrModelName = model_name;
	
	lua_rawgeti(L, -1, 2);
	bool malformed = false;
	if (lua_istable(L, -1)) {
		for (int i=0; i<THRUSTER_MAX; i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1)) {
				s.linThrust[i] = lua_tonumber(L, -1);
				lua_pop(L, 1);
			} else {
				malformed = true;
				lua_pop(L, 1);
				break;
			}
		}
	} else {
		malformed = true;
	}
	lua_pop(L, 1);
	if (malformed) Error("Expected thruster values in ship_defs of %s\n", model_name);

	s.angThrust = (float)luaPi_gettable_checknumber(L, -1, 3);

	lua_rawgeti(L, -1, 4);
	if (lua_istable(L, -1)) {
		for (unsigned int i=0; i<lua_objlen(L,-1); i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			if (lua_istable(L, -1) && lua_objlen(L,-2) == 2)	{
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				s.gunMount[i].pos = *MyLuaVec::checkVec(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				s.gunMount[i].dir = *MyLuaVec::checkVec(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	for (int i=0; i<Equip::SLOT_MAX; i++) s.equipSlotCapacity[i] = 0;
	lua_rawgeti(L, -1, 5);
	if (lua_istable(L, -1)) {
		for (unsigned int i=0; (i<lua_objlen(L,-1)) && (i<Equip::SLOT_MAX); i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			s.equipSlotCapacity[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	s.capacity = luaPi_gettable_checkinteger(L, -1, 6);
	s.hullMass = luaPi_gettable_checkinteger(L, -1, 7);
	s.baseprice = luaPi_gettable_checkinteger(L, -1, 8);
	lua_rawgeti(L, -1, 9);
	if(!lua_isnoneornil(L, -1)) {
		s.hyperdrive = (Equip::Type)((int)Equip::DRIVE_CLASS1+luaL_checkinteger(L, -1)-1);
	} else {
		s.hyperdrive = Equip::NONE;
	}
	lua_pop(L, 1);

	types[s.name] = s;
	return 0;
}

void ShipType::Init()
{
	static bool isInitted = false;
	if (isInitted) return;
	isInitted = true;

	std::vector<LmrModel*> ship_models;
	LmrGetModelsWithTag("ship", ship_models);
	lua_State *L = LmrGetLuaState();
	int num = 0;

	for (std::vector<LmrModel*>::iterator i = ship_models.begin();
			i != ship_models.end(); ++i) {
		LmrModel *model = *i;
		model->PushAttributeToLuaStack("ship_defs");
		if (lua_isnil(L, -1)) {
			Error("Model %s is tagged as ship but has no ship_defs.",
					model->GetName());
		} else if (lua_istable(L, -1)) {
			// multiple ship-defs for 1 model
			for (unsigned int i=0; i<lua_objlen(L,-1); i++) {
				lua_pushinteger(L, i+1);
				lua_gettable(L, -2);
				define_ship(L, model->GetName());
				num++;
				lua_pop(L, 1);
			}
		} else {
			Error("Model %s: ships_def is malformed", model->GetName());
		}
		lua_pop(L, 1);
	}
	printf("%d ship types.\n", num);
}

ShipType::Type ShipType::GetRandomType() {
	ShipType::Type type = "";
	while (true) {
		std::map<ShipType::Type, ShipType>::iterator iter = types.begin();
		int idx = Pi::rng.Int32(types.size());
		for (int i=0; i<=idx; i++) {
			type = iter->first;
			iter++;
		}
		// Don't include missiles
		if (type.find("MISSILE")!=0)
			break;
	}
	return type;
}

void EquipSet::Save(Serializer::Writer &wr)
{
	wr.Int32(Equip::SLOT_MAX);
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		wr.Int32(equip[i].size());
		for (unsigned int j=0; j<equip[i].size(); j++) {
			wr.Int32(static_cast<int>(equip[i][j]));
		}
	}
}

/*
 * Should have initialised with EquipSet(ShipType::Type) first
 */
void EquipSet::Load(Serializer::Reader &rd)
{
	const int numSlots = rd.Int32();
	assert(numSlots <= Equip::SLOT_MAX);
	for (int i=0; i<numSlots; i++) {
		const int numItems = rd.Int32();
		assert(numItems <= (signed)equip[i].size());
		for (int j=0; j<numItems; j++) {
			equip[i][j] = static_cast<Equip::Type>(rd.Int32());
		}
	}
	onChange.emit();
}

