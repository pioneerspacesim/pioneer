// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sensors.h"
#include "Body.h"
#include "Game.h"
#include "HudTrail.h"
#include "Pi.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"

Sensors::RadarContact::RadarContact() :
	body(0),
	trail(0),
	distance(0.0),
	iff(IFF_UNKNOWN),
	fresh(true)
{
}

Sensors::RadarContact::RadarContact(Body *b) :
	body(b),
	trail(0),
	distance(0.0),
	iff(IFF_UNKNOWN),
	fresh(true)
{
}

Sensors::RadarContact::~RadarContact()
{
	body = 0;
	delete trail;
}

Color Sensors::IFFColor(IFF iff)
{
	switch (iff) {
	case IFF_NEUTRAL: return Color::BLUE;
	case IFF_ALLY: return Color::GREEN;
	case IFF_HOSTILE: return Color::RED;
	case IFF_UNKNOWN:
	default:
		return Color::GRAY;
	}
}

bool Sensors::ContactDistanceSort(const RadarContact &a, const RadarContact &b)
{
	return a.distance < b.distance;
}

Sensors::Sensors(Ship *owner)
{
	m_owner = owner;
}

Body* Sensors::ChooseTarget(TargetingCriteria crit, const Body* oldTarget )
{
	PROFILE_SCOPED();

	if(!m_owner->IsType(ObjectType::PLAYER))
		return nullptr;
		
	const Body* currTarget = oldTarget;

	m_radarContacts.sort(ContactDistanceSort);

	for (auto it = m_radarContacts.begin(); it != m_radarContacts.end(); ++it) {
		//match object type
		//match iff
		if (it->body->IsType(ObjectType::SHIP)) {

			if(crit == CYCLE_HOSTILE && currTarget) {
				if(currTarget == it->body) {
					currTarget = nullptr;
					//next hostile will be selected
				}
				continue;
			}

			if (it->iff != IFF_HOSTILE) continue;
			//should move the target to ship after all (from PlayerShipController)
			//targeting inputs stay in PSC

			return it->body;
		}
	}

	return nullptr;
}

Sensors::IFF Sensors::CheckIFF(Body *other)
{
	PROFILE_SCOPED();
	//complicated relationship check goes here
	if (other->IsType(ObjectType::SHIP)) {
		Ship* ship = static_cast<Ship*>(other);
		Uint8 rel = m_owner->GetRelations(ship);
		if (rel == 0 || ship->IsAIAttacking(m_owner))
		{
			return IFF_HOSTILE;
		}
		else if (rel == 100)
			return IFF_ALLY;
		return IFF_NEUTRAL;
	} else {
		return IFF_UNKNOWN;
	}
}

void Sensors::Update(float time)
{
	PROFILE_SCOPED();
	if (m_owner != Pi::player) return;

	PopulateStaticContacts(); //no need to do all the time

	//Find nearby contacts, same range as radar scanner. It should use these
	//contacts, worldview labels too.
	Space::BodyNearList nearby = Pi::game->GetSpace()->GetBodiesMaybeNear(m_owner, 100000.0f);
	for (Body *body : nearby) {
		if (body == m_owner || !body->IsType(ObjectType::SHIP)) continue;
		if (body->IsDead()) continue;

		auto cit = m_radarContacts.begin();
		while (cit != m_radarContacts.end()) {
			if (cit->body == body) break;
			++cit;
		}

		//create new contact or refresh old
		if (cit == m_radarContacts.end()) {
			m_radarContacts.push_back(RadarContact());
			RadarContact &rc = m_radarContacts.back();
			rc.body = body;
			rc.iff = CheckIFF(rc.body);
			rc.trail = new HudTrail(rc.body, IFFColor(rc.iff));
		} else {
			cit->fresh = true;
		}
	}

	//update contacts and delete stale ones
	auto it = m_radarContacts.begin();
	while (it != m_radarContacts.end()) {
		if (!it->fresh) {
			m_radarContacts.erase(it++);
		} else {
			const Ship *ship = it->body->IsType(ObjectType::SHIP) ? static_cast<Ship *>(it->body) : nullptr;
			if (ship && Ship::FLYING == ship->GetFlightState()) {
				it->distance = m_owner->GetPositionRelTo(it->body).Length();
				it->iff = CheckIFF(it->body);
				it->trail->SetColor(IFFColor(it->iff));
				it->trail->Update(time);
			} else {
				it->trail->Reset(FrameId::Invalid);
			}
			it->fresh = false;
			++it;
		}
	}
}

void Sensors::UpdateIFF(Body *b)
{
	PROFILE_SCOPED();
	for (auto it = m_radarContacts.begin(); it != m_radarContacts.end(); ++it) {
		if (it->body == b) {
			it->iff = CheckIFF(b);
			it->trail->SetColor(IFFColor(it->iff));
		}
	}
}

void Sensors::ResetTrails()
{
	PROFILE_SCOPED();
	for (auto it = m_radarContacts.begin(); it != m_radarContacts.end(); ++it)
		it->trail->Reset(Pi::player->GetFrame());
}

void Sensors::PopulateStaticContacts()
{
	PROFILE_SCOPED();
	m_staticContacts.clear();

	for (Body *b : Pi::game->GetSpace()->GetBodies()) {
		switch (b->GetType()) {
		case ObjectType::STAR:
		case ObjectType::PLANET:
		case ObjectType::SPACESTATION:
			break;
		default:
			continue;
		}
		m_staticContacts.push_back(RadarContact(b));
		RadarContact &rc = m_staticContacts.back();
		rc.fresh = true;
	}
}
