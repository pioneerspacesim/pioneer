#include <math.h>
#include "Turret.h"
#include "Game.h"
#include "Pi.h"
#include "Ship.h"
#include "ShipType.h"
#include "Body.h"
#include "EquipType.h"
#include "Projectile.h"
#include "Polit.h"
#include "Sound.h"

Turret::Turret(Ship *parent, const TurretData &turret) :
	m_parent(parent),
	m_weapontype(Equip::NONE),
	m_coolrate(0.01f),
	m_firing(false),
	m_temperature(0.f),
	m_recharge(0.f),
	m_turret(turret),
	m_curvel(0.0, 0.0, 0.0),
	m_curdir(turret.dir),
	m_skill(0.5f),
	m_target(0),
	m_dotextent(cos(turret.extent))
{
}

void Turret::Save(Serializer::Writer &wr)
{
	wr.Bool(m_firing);
	wr.Float(m_temperature);
	wr.Float(m_recharge);
	wr.Vector3d(m_curvel);
	wr.Vector3d(m_curdir);
	wr.Float(m_skill);			// not sure how this is going to work
}

void Turret::Load(Serializer::Reader &rd)
{
	m_firing = rd.Bool();
	m_temperature = rd.Float();
	m_recharge = rd.Float();
	m_curvel = rd.Vector3d();
	m_curdir = rd.Vector3d();
	m_skill = rd.Float();
}

void Turret::SetWeapon(Equip::Type weapontype, float coolfactor)
{
	m_coolrate = 0.01f * coolfactor;
	m_weapontype = weapontype;
}

void Turret::Update(float timeStep)
{
	return;
#if 0
	if (!m_target || m_weapontype == Equip::NONE) return;

	// if turret wasn't manually controlled, run the autotarget routine
	AutoTarget(timeStep);

	double ang = m_curvel.Length() * timeStep;
	// XXX make proper rotation matrix instead, saves some sqrts
	if (ang > 1e-16) m_curdir.ArbRotate(m_curvel / ang, ang);

	// clamp to turret extent
	if (m_curdir.Dot(m_turret.dir) < m_dotextent)
	{
		vector3d raxis = m_turret.dir.Cross(m_curdir);
		m_curdir = m_turret.dir;
		m_curdir.ArbRotate(raxis, m_turret.extent);
		m_curvel -= m_turret.dir * m_curvel.Dot(m_turret.dir);
	}

	m_recharge -= timeStep;
	m_temperature -= m_coolrate*timeStep;
	m_temperature = std::max(m_temperature, 0.0f);
	m_recharge = std::max(m_recharge, 0.0f);

	if (!m_firing || m_recharge > 0.0f || m_temperature > 1.0f) return;

	const matrix3x3d &rot = m_parent->GetOrient();
	const vector3d dir = rot * GetDir();
	const vector3d pos = rot * m_turret.pos + m_parent->GetPosition();

	const LaserType &lt = Equip::lasers[Equip::types[m_weapontype].tableIndex];
	m_temperature += 0.01f;			// XXX should be weapon dependent?
	m_recharge += lt.rechargeTime;
	vector3d baseVel = m_parent->GetVelocity();
	vector3d dirVel = lt.speed * dir;

	Projectile::Add(m_parent, m_weapontype, pos, baseVel, dirVel);

	Polit::NotifyOfCrime(m_parent, Polit::CRIME_WEAPON_DISCHARGE);
	Sound::BodyMakeNoise(m_parent, "Pulse_Laser", 1.0f);
#endif
}

extern double calc_ivel(double dist, double vel, double acc);

// av is in object space
void Turret::MatchAngVelInternal(const vector3d &av)
{
	double frameAccel = m_turret.accel * Pi::game->GetTimeStep();
	vector3d diffvel = (av - m_curvel);
	if (diffvel.Length() > frameAccel) diffvel *= frameAccel / diffvel.Length();
	m_curvel += diffvel;
}

void Turret::FaceDirectionInternal(const vector3d &dir, double leadAV)
{
	vector3d dav(0.0, 0.0, 0.0);	// desired angular velocity
	double dp = dir.Dot(m_curdir);
	if (dp < 0.999999) {
		double ang = acos(Clamp(dp, -1.0, 1.0));
		double iangspeed = leadAV + calc_ivel(ang, 0.0, m_turret.accel);
		iangspeed = std::min(iangspeed, m_turret.maxspeed);
		dav = iangspeed * m_curdir.Cross(dir).Normalized();
	}
	MatchAngVelInternal(dav);
}

void Turret::AutoTarget(float timeStep)
{
	const matrix3x3d &rot = m_parent->GetOrient();				// some world-space params
	const vector3d targpos = m_target->GetPositionRelTo(m_parent);
	//const vector3d targvel = m_target->GetVelocityRelTo(m_parent);
	const vector3d targdir = targpos.NormalizedSafe();
	const vector3d heading = rot * m_curdir;

// XXX: Put last acceleration value in body? It's quite useful...
	//	vector3d targaccel = (targvel - m_lastVel) / timeStep;
	const vector3d leaddir = m_parent->AIGetLeadDir(m_target, vector3d(0.0), m_weapontype);

	// turn towards target lead direction, add inaccuracy
	// trigger recheck when angular velocity reaches zero or after certain time

	if (m_leadTime < Pi::game->GetTime())
	{
		const double headdiff = (leaddir - heading).Length();
		const double leaddiff = (leaddir - targdir).Length();
		m_leadTime = Pi::game->GetTime() + headdiff + (1.0*Pi::rng.Double()*m_skill);

		// lead inaccuracy based on diff between heading and leaddir
		vector3d r(Pi::rng.Double()-0.5, Pi::rng.Double()-0.5, Pi::rng.Double()-0.5);
		vector3d newoffset = r * (0.02 + 2.0*leaddiff + 2.0*headdiff)*Pi::rng.Double()*m_skill;
		m_leadOffset = (heading - leaddir);		// should be already...
		m_leadDrift = (newoffset - m_leadOffset) / (m_leadTime - Pi::game->GetTime());

		// Shoot only when close to target

		double vissize = 1.3 * m_target->GetClipRadius() / targpos.Length();
		vissize += (0.05 + 0.5*leaddiff)*Pi::rng.Double()*m_skill;
		if (vissize > headdiff) 
			SetFiring(true);
		else 
			SetFiring(false);

		if (targpos.LengthSqr() > 4000*4000) 
			SetFiring(false);
	}
	m_leadOffset += m_leadDrift * timeStep;
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).NormalizedSafe());	// leaddir angvel
	vector3d facedir = (leaddir + m_leadOffset).Normalized();

	FaceDirectionInternal(facedir, leadAV);
}
