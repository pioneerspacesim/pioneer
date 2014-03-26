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

#pragma optimize("",off)
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
	m_dotextent(cos(turret.extent)),
	m_manual(false)
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
	if (!m_manual && (!m_target || m_weapontype == Equip::NONE)) return;

	// if turret wasn't manually controlled, run the autotarget routine
	if (!m_manual) AutoTarget(timeStep);

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
	m_manual = false;			// clear for next timestep

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

/*	if (lt.flags & Equip::LASER_DUAL)
	{
		const ShipType::DualLaserOrientation orient = stype.gunMount[num].orient;
		const vector3d orient_norm =
				(orient == ShipType::DUAL_LASERS_VERTICAL) ? vector3d(m[0],m[1],m[2]) : vector3d(m[4],m[5],m[6]);
		const vector3d sep = stype.gunMount[num].sep * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add(this, t, pos + sep, baseVel, dirVel);
		Projectile::Add(this, t, pos - sep, baseVel, dirVel);
	}
	else
*/

	/*
			// trace laser beam through frame to see who it hits
			CollisionContact c;
			GetFrame()->GetCollisionSpace()->TraceRay(pos, dir, 10000.0, &c, this->GetGeom());
			if (c.userData1) {
				Body *hit = static_cast<Body*>(c.userData1);
				hit->OnDamage(this, damage);
			}
	*/

	Polit::NotifyOfCrime(m_parent, Polit::CRIME_WEAPON_DISCHARGE);
	Sound::BodyMakeNoise(m_parent, "Pulse_Laser", 1.0f);
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

// returns dir clamp to turret extent
// direction is in object space
vector3d Turret::FaceDirection(const vector3d &dir)
{
	vector3d clampdir =	dir;
	if (clampdir.Dot(m_turret.dir) < m_dotextent)			// clamp to extent
	{
		vector3d raxis = m_turret.dir.Cross(clampdir);
		clampdir = m_turret.dir;
		clampdir.ArbRotate(raxis, m_turret.extent);
	}
	m_manual = true;
	FaceDirectionInternal(clampdir, 0.0);
	return clampdir;
}

void Turret::MatchAngVel(const vector3d &av)
{
	m_manual = true;
	MatchAngVelInternal(GetOrient() * av);
}

void Turret::AutoTarget(float timeStep)
{
	const matrix3x3d &rot = m_parent->GetOrient();				// some world-space params
	vector3d targpos = m_target->GetPositionRelTo(m_parent);
	vector3d targvel = m_target->GetVelocityRelTo(m_parent);
	vector3d targdir = targpos.NormalizedSafe();
	vector3d heading = rot * m_curdir;

// XXX: Put last acceleration value in body? It's quite useful...
	//	vector3d targaccel = (targvel - m_lastVel) / timeStep;
	vector3d leaddir = m_parent->AIGetLeadDir(m_target, vector3d(0.0), m_weapontype);

	// turn towards target lead direction, add inaccuracy
	// trigger recheck when angular velocity reaches zero or after certain time

	if (m_leadTime < Pi::game->GetTime())
	{
		double headdiff = (leaddir - heading).Length();
		double leaddiff = (leaddir - targdir).Length();
		m_leadTime = Pi::game->GetTime() + headdiff + (1.0*Pi::rng.Double()*m_skill);

		// lead inaccuracy based on diff between heading and leaddir
		vector3d r(Pi::rng.Double()-0.5, Pi::rng.Double()-0.5, Pi::rng.Double()-0.5);
		vector3d newoffset = r * (0.02 + 2.0*leaddiff + 2.0*headdiff)*Pi::rng.Double()*m_skill;
		m_leadOffset = (heading - leaddir);		// should be already...
		m_leadDrift = (newoffset - m_leadOffset) / (m_leadTime - Pi::game->GetTime());

		// Shoot only when close to target

		double vissize = 1.3 * m_target->GetClipRadius() / targpos.Length();
		vissize += (0.05 + 0.5*leaddiff)*Pi::rng.Double()*m_skill;
		if (vissize > headdiff) SetFiring(true);
		else SetFiring(false);
		if (targpos.LengthSqr() > 4000*4000) SetFiring(false);
	}
	m_leadOffset += m_leadDrift * timeStep;
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).NormalizedSafe());	// leaddir angvel
	vector3d facedir = (leaddir + m_leadOffset).Normalized();

	FaceDirectionInternal(facedir, leadAV);
}

matrix3x3d Turret::GetOrient() const
{
	vector3d zaxis = -m_turret.dir;
	vector3d yaxis = zaxis.Cross(vector3d(0.0,1.0,0.0)).Cross(zaxis).NormalizedSafe();
	vector3d xaxis = yaxis.Cross(zaxis);
	matrix3x3d base = matrix3x3d::FromVectors(xaxis, yaxis, zaxis);

	double dp = m_curdir.Dot(m_turret.dir);
	if (dp < 0.999999) {
		double ang = acos(Clamp(dp, -1.0, 1.0));
		vector3d axis = m_turret.dir.Cross(m_curdir).Normalized();
		base = matrix3x3d::Rotate(ang, axis) * base;
	}
	return base;
}
