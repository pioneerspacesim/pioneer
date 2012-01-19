#include "libs.h"
#include "Pi.h"
#include "Projectile.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "render/Render.h"
#include "CargoBody.h"
#include "Planet.h"
#include "Sfx.h"
#include "Ship.h"
#include "TextureCache.h"
#include "Pi.h"
#include "Game.h"

Projectile::Projectile(): Body()
{
	m_orient = matrix4x4d::Identity();
	m_type = 1;
	m_age = 0;
	m_parent = 0;
	m_flags |= FLAG_DRAW_LAST;
}

void Projectile::Save(Serializer::Writer &wr, Space *space)
{
	Body::Save(wr, space);
	for (int i=0; i<16; i++) wr.Double(m_orient[i]);
	wr.Vector3d(m_baseVel);
	wr.Vector3d(m_dirVel);
	wr.Float(m_age);
	wr.Int32(m_type);
	wr.Int32(space->GetIndexForBody(m_parent));
}

void Projectile::Load(Serializer::Reader &rd, Space *space)
{
	Body::Load(rd, space);
	for (int i=0; i<16; i++) m_orient[i] = rd.Double();
	m_baseVel = rd.Vector3d();
	m_dirVel = rd.Vector3d();
	m_age = rd.Float();
	m_type = rd.Int32();
	m_parentIndex = rd.Int32();
}

void Projectile::PostLoadFixup(Space *space)
{
	m_parent = space->GetBodyByIndex(m_parentIndex);
}

void Projectile::UpdateInterpolatedTransform(double alpha)
{
	m_interpolatedTransform = m_orient;
	const vector3d newPos = GetPosition();
	const vector3d oldPos = newPos - (m_baseVel+m_dirVel)*Pi::game->GetTimeStep();
	const vector3d p = alpha*newPos + (1.0-alpha)*oldPos;
	m_interpolatedTransform[12] = p.x;
	m_interpolatedTransform[13] = p.y;
	m_interpolatedTransform[14] = p.z;
}

void Projectile::SetPosition(vector3d p)
{
	m_orient[12] = p.x;
	m_orient[13] = p.y;
	m_orient[14] = p.z;
}

void Projectile::NotifyRemoved(const Body* const removedBody)
{
	if (m_parent == removedBody) m_parent = 0;
}

void Projectile::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	SetPosition(GetPosition() + (m_baseVel+m_dirVel) * double(timeStep));
	if (m_age > Equip::lasers[m_type].lifespan) Pi::game->GetSpace()->KillBody(this);
}

/* In hull kg */
float Projectile::GetDamage() const
{
	float dam = Equip::lasers[m_type].damage;
	float lifespan = Equip::lasers[m_type].lifespan;
	return dam * sqrt((lifespan - m_age)/lifespan);
	// TEST
//	return 0.01f;
}

static void MiningLaserSpawnTastyStuff(Frame *f, const SBody *asteroid, const vector3d &pos)
{
	Equip::Type t;
	if (20*Pi::rng.Fixed() < asteroid->m_metallicity) {
		t = Equip::PRECIOUS_METALS;
	} else if (8*Pi::rng.Fixed() < asteroid->m_metallicity) {
		t = Equip::METAL_ALLOYS;
	} else if (Pi::rng.Fixed() < asteroid->m_metallicity) {
		t = Equip::METAL_ORE;
	} else if (Pi::rng.Fixed() < fixed(1,2)) {
		t = Equip::WATER;
	} else {
		t = Equip::RUBBISH;
	}
	CargoBody *cargo = new CargoBody(t);
	cargo->SetFrame(f);
	cargo->SetPosition(pos);
	cargo->SetVelocity(Pi::rng.Double(100.0,200.0)*vector3d(Pi::rng.Double()-.5, Pi::rng.Double()-.5, Pi::rng.Double()-.5));
	Pi::game->GetSpace()->AddBody(cargo);
}

void Projectile::StaticUpdate(const float timeStep)
{
	CollisionContact c;
	vector3d vel = m_dirVel * 0.1;
	GetFrame()->GetCollisionSpace()->TraceRay(GetPosition(), vel.Normalized(), vel.Length(), &c, 0);
	
	if (c.userData1) {
		Object *o = static_cast<Object*>(c.userData1);

		if (o->IsType(Object::CITYONPLANET)) {
			Pi::game->GetSpace()->KillBody(this);
		}
		else if (o->IsType(Object::BODY)) {
			Body *hit = static_cast<Body*>(o);
			if (hit != m_parent) {
				hit->OnDamage(m_parent, GetDamage());
				Pi::game->GetSpace()->KillBody(this);
				if (hit->IsType(Object::SHIP))
					Pi::luaOnShipHit->Queue(dynamic_cast<Ship*>(hit), dynamic_cast<Body*>(m_parent));
			}
		}
	}
	if (Equip::lasers[m_type].flags & Equip::LASER_MINING) {
		// need to test for terrain hit
		if (GetFrame()->m_astroBody && GetFrame()->m_astroBody->IsType(Object::PLANET)) {
			Planet *const planet = static_cast<Planet*>(GetFrame()->m_astroBody);
			const SBody *b = planet->GetSBody();
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				// hit the fucker
				if (b->type == SBody::TYPE_PLANET_ASTEROID) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), b, n*terrainHeight + 5.0*n);
					Sfx::Add(this, Sfx::TYPE_EXPLOSION);
				}
				Pi::game->GetSpace()->KillBody(this);
			}
		}
	}
}

void Projectile::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	ModelTexture *tex = Pi::textureCache->GetModelTexture(PIONEER_DATA_DIR"/textures/laser.png");

	vector3d from = viewTransform * GetInterpolatedPosition();
	vector3d to = viewTransform * (GetInterpolatedPosition() + 0.1*m_dirVel);
	vector3d dir = to - from;

	vector3f _from(&from.x);
	vector3f _dir(&dir.x);
	vector3f points[50];
	float p = 0;
	for (int i=0; i<50; i++, p+=0.02) {
		points[i] = _from + p*_dir;
	}
	Color col = Equip::lasers[m_type].color;
	col.a = 1.0f - m_age/Equip::lasers[m_type].lifespan;
	tex->Bind();
	Render::PutPointSprites(50, points, Equip::lasers[m_type].psize, col);
}

void Projectile::Add(Body *parent, Equip::Type type, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel)
{
	Projectile *p = new Projectile();
	p->m_parent = parent;
	p->m_type = Equip::types[type].tableIndex;
	p->SetFrame(parent->GetFrame());
	
	parent->GetRotMatrix(p->m_orient);
	p->SetPosition(pos);
	p->m_baseVel = baseVel;
	p->m_dirVel = dirVel;
	Pi::game->GetSpace()->AddBody(p);
}
