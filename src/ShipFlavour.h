// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"
#include "Serializer.h"

struct LmrObjParams;
struct lua_State;

class ShipFlavour {
public:
	ShipType::Id id;
	std::string regid;
	int price;

	static ShipFlavour FromLuaTable(lua_State *l, int idx);

	ShipFlavour();
	ShipFlavour(ShipType::Id id);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	void ApplyTo(ModelBase *m) const;
	static void MakeTrulyRandom(ShipFlavour &v, bool atmosphereCapableOnly = false);
};


#endif /* _SHIPFLAVOUR_H */
