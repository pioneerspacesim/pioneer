#include "Player.h"
#include "Frame.h"
#include "Game.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "Pi.h"
#include "SectorView.h"
#include "Serializer.h"
#include "ShipCpanel.h"
#include "Sound.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "WorldView.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	SetController(new PlayerShipController());
	m_killCount = 0;
	m_knownKillCount = 0;
	UpdateMass();
}

void Player::Save(Serializer::Writer &wr, Space *space)
{
	Ship::Save(wr, space);
	MarketAgent::Save(wr);
	wr.Int32(m_killCount);
	wr.Int32(m_knownKillCount);
}

void Player::Load(Serializer::Reader &rd, Space *space)
{
	Pi::player = this;
	Ship::Load(rd, space);
	MarketAgent::Load(rd);
	m_killCount = rd.Int32();
	m_knownKillCount = rd.Int32();
}

void Player::OnHaveKilled(Body *guyWeKilled)
{
	if (guyWeKilled->IsType(Object::SHIP)) {
		printf("Well done. you killed some poor fucker\n");
		m_killCount++;
	}
}

bool Player::OnDamage(Object *attacker, float kgDamage)
{
	bool r = Ship::OnDamage(attacker, kgDamage);
	if (!IsDead() && (GetPercentHull() < 25.0f)) {
		Sound::BodyMakeNoise(this, "warning", .5f);
	}
	return r;
}

void Player::Render(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (!IsDead()) Ship::Render(r, viewCoords, viewTransform);
}

void Player::SetDockedWith(SpaceStation *s, int port)
{
	Ship::SetDockedWith(s, port);
	if (s) {
		if (Pi::CombatRating(m_killCount) > Pi::CombatRating(m_knownKillCount)) {
			Pi::cpan->MsgLog()->ImportantMessage(Lang::PIONEERING_PILOTS_GUILD, Lang::RIGHT_ON_COMMANDER);
		}
		m_knownKillCount = m_killCount;

		Pi::SetView(Pi::spaceStationView);
	}
}

bool Player::SetWheelState(bool down)
{
	static Sound::Event sndev;
	bool did = Ship::SetWheelState(down);
	if (did) {
		sndev.Play(down ? "UC_out" : "UC_in", 1.0f, 1.0f, 0);
	}
	return did;
}

bool Player::FireMissile(int idx, Ship *target)
{
	if (!Ship::FireMissile(idx, target))
		return false;
	
	Sound::PlaySfx("Missile_launch", 1.0f, 1.0f, 0);
	return true;
}

void Player::SetAlertState(Ship::AlertState as)
{
	Ship::AlertState prev = GetAlertState();

	switch (as) {
		case ALERT_NONE:
			if (prev != ALERT_NONE)
				Pi::cpan->MsgLog()->Message("", Lang::ALERT_CANCELLED);
			break;

		case ALERT_SHIP_NEARBY:
			if (prev == ALERT_NONE)
				Pi::cpan->MsgLog()->ImportantMessage("", Lang::SHIP_DETECTED_NEARBY);
			else
				Pi::cpan->MsgLog()->ImportantMessage("", Lang::DOWNGRADING_ALERT_STATUS);
			Sound::PlaySfx("OK");
			break;

		case ALERT_SHIP_FIRING:
			Pi::cpan->MsgLog()->ImportantMessage("", Lang::LASER_FIRE_DETECTED);
			Sound::PlaySfx("warning", 0.2f, 0.2f, 0);
			break;
	}

	Pi::cpan->SetAlertState(as);

	Ship::SetAlertState(as);
}

void Player::NotifyRemoved(const Body* const removedBody)
{
	if (GetNavTarget() == removedBody)
		SetNavTarget(0);

	else if (GetCombatTarget() == removedBody) {
		SetCombatTarget(0);

		if (!GetNavTarget() && removedBody->IsType(Object::SHIP))
			SetNavTarget(static_cast<const Ship*>(removedBody)->GetHyperspaceCloud());
	}

	Ship::NotifyRemoved(removedBody);
}

/* MarketAgent shite */
void Player::Bought(Equip::Type t)
{
	m_equipment.Add(t);
	UpdateMass();
}

void Player::Sold(Equip::Type t)
{
	m_equipment.Remove(t, 1);
	UpdateMass();
}

bool Player::CanBuy(Equip::Type t, bool verbose) const
{
	Equip::Slot slot = Equip::types[int(t)].slot;
	bool freespace = (m_equipment.FreeSpace(slot)!=0);
	bool freecapacity = (m_stats.free_capacity >= Equip::types[int(t)].mass);
	if (verbose) {
		if (!freespace) {
			Pi::Message(Lang::NO_FREE_SPACE_FOR_ITEM);
		}
		else if (!freecapacity) {
			Pi::Message(Lang::SHIP_IS_FULLY_LADEN);
		}
	}
	return (freespace && freecapacity);
}

bool Player::CanSell(Equip::Type t, bool verbose) const
{
	Equip::Slot slot = Equip::types[int(t)].slot;
	bool cansell = (m_equipment.Count(slot, t) > 0);
	if (verbose) {
		if (!cansell) {
			Pi::Message(stringf(Lang::YOU_DO_NOT_HAVE_ANY_X, formatarg("item", Equip::types[int(t)].name)));
		}
	}
	return cansell;
}

Sint64 Player::GetPrice(Equip::Type t) const
{
	if (Ship::GetDockedWith()) {
		return Ship::GetDockedWith()->GetPrice(t);
	} else {
		assert(0);
		return 0;
	}
}

void Player::OnEnterHyperspace()
{
	SetNavTarget(0);
	SetCombatTarget(0);

	Pi::worldView->HideTargetActions(); // hide the comms menu
	m_controller->SetFlightControlState(CONTROL_MANUAL); //could set CONTROL_HYPERDRIVE
	ClearThrusterState();
	Pi::game->WantHyperspace();
}

void Player::OnEnterSystem()
{
	m_controller->SetFlightControlState(CONTROL_MANUAL);
	Pi::sectorView->ResetHyperspaceTarget();
}

#pragma region tempstuff
void Player::SetMouseForRearView(bool enable)
{
	static_cast<PlayerShipController*>(m_controller)->SetMouseForRearView(enable);
}

bool Player::IsMouseActive() const
{
	return static_cast<PlayerShipController*>(m_controller)->IsMouseActive();
}

vector3d Player::GetMouseDir() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetMouseDir();
}

FlightControlState Player::GetFlightControlState() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetFlightControlState();
}

void Player::SetFlightControlState(enum FlightControlState s)
{
	static_cast<PlayerShipController*>(m_controller)->SetFlightControlState(s);
}

double Player::GetSetSpeed() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetSetSpeed();
}

Body *Player::GetCombatTarget() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetCombatTarget();
}

Body *Player::GetNavTarget() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetNavTarget();
}

Body *Player::GetSetSpeedTarget() const
{
	return static_cast<PlayerShipController*>(m_controller)->GetSetSpeedTarget();
}

void Player::SetCombatTarget(Body* const target, bool setSpeedTo)
{
	static_cast<PlayerShipController*>(m_controller)->SetCombatTarget(target, setSpeedTo);
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Player::SetNavTarget(Body* const target, bool setSpeedTo)
{
	static_cast<PlayerShipController*>(m_controller)->SetNavTarget(target, setSpeedTo);
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}
#pragma endregion
