// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "utils.h"
#include "ShipType.h"
#include "ShipFlavour.h"
#include "Pi.h"
#include "Serializer.h"
#include "scenegraph/SceneGraph.h"

ShipFlavour::ShipFlavour()
{
}

ShipFlavour::ShipFlavour(ShipType::Id id_)
{
	id = id_;
}

// Pick a random ship type, and randomize the flavour
void ShipFlavour::MakeTrulyRandom(ShipFlavour &v, bool atmospheric)
{
	// only allow ships that can fit an atmospheric shield
	if (atmospheric) {
		const std::vector<ShipType::Id> &ships = ShipType::playable_atmospheric_ships;
		v = ShipFlavour(ships[Pi::rng.Int32(ships.size())]);
	} else {
		const std::vector<ShipType::Id> &ships = ShipType::player_ships;
		v = ShipFlavour(ships[Pi::rng.Int32(ships.size())]);
	}
}

void ShipFlavour::Save(Serializer::Writer &wr)
{
	wr.String(id);
}

void ShipFlavour::Load(Serializer::Reader &rd)
{
	id = rd.String();
}
