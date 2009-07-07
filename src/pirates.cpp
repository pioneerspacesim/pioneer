#include "libs.h"
#include "Pi.h"
#include "Ship.h"
#include "Space.h"
#include "Frame.h"
#include "Player.h"

static void spawn_random_pirate(int power, Ship *victim)
{
	float longitude = (float)Pi::rng.Double(M_PI);
	float latitude = (float)Pi::rng.Double(M_PI);
	float dist = 2000.0f;
	vector3d relpos(vector3d(sin(longitude)*cos(latitude)*dist,
			sin(latitude)*dist,
			cos(longitude)*cos(latitude)*dist));

	ShipType::Type t;

	switch(Pi::rng.Int32(5) + power) {
		case 0: case 1:
		case 2: case 3:
			t = ShipType::LADYBIRD;
			break;
		default:
			t = ShipType::SIRIUS_INTERDICTOR;
			break;
	}

	Ship *ship = new Ship(t);
	ship->AIInstruct(Ship::DO_KILL, victim);
	ship->SetFrame(victim->GetFrame());
	ship->SetPosition(victim->GetPosition() + relpos);
	ship->SetVelocity(victim->GetVelocity());
	Space::AddBody(ship);
	
	ship->m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS1);

	switch (power) {
		case 1:
			ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::LASER_2MW_BEAM);
			break;
		case 2:
			ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::LASER_4MW_BEAM);
			break;
		case 0:
		default:
			ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::LASER_1MW_BEAM);
			break;
	}
	int amount = Pi::rng.Int32(5);
	while (amount--) ship->m_equipment.Add(Equip::SLOT_CARGO, Equip::HYDROGEN);
}

void SpawnPiratesOnHyperspace()
{
	if (Pi::rng.Int32(3)==0) {
		int num_pirates = Pi::rng.Int32(4);
		while (num_pirates--) spawn_random_pirate(Pi::rng.Int32(1,3), Pi::player);
	}
}

