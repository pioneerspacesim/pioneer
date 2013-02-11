// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPFLAVOUR_H
#define _SHIPFLAVOUR_H

#include "ShipType.h"
#include "Serializer.h"

struct lua_State;
namespace SceneGraph { class Model; }

class ShipFlavour {
public:
	ShipType::Id id;

	ShipFlavour();
	ShipFlavour(ShipType::Id id);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	static void MakeTrulyRandom(ShipFlavour &v, bool atmosphereCapableOnly = false);
};


#endif /* _SHIPFLAVOUR_H */
