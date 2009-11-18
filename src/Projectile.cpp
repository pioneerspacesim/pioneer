#include "libs.h"
#include "Pi.h"
#include "Projectile.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Render.h"
#include "Shader.h"
#include "CargoBody.h"
#include "Planet.h"
#include "Sfx.h"

#define PROJECTILE_AGE 1.0f

Projectile::Projectile(): Body()
{
	m_orient = matrix4x4d::Identity();
	m_type = TYPE_1MW_PULSE;
	m_age = 0;
	m_parent = 0;
}

void Projectile::Save()
{
	using namespace Serializer::Write;
	Body::Save();
	for (int i=0; i<16; i++) wr_double(m_orient[i]);
	wr_vector3d(m_baseVel);
	wr_vector3d(m_dirVel);
	wr_float(m_age);
	wr_int(m_type);
	wr_int(Serializer::LookupBody(m_parent));
}

void Projectile::Load()
{
	using namespace Serializer::Read;
	Body::Load();
	for (int i=0; i<16; i++) m_orient[i] = rd_double();
	m_baseVel = rd_vector3d();
	m_dirVel = rd_vector3d();
	m_age = rd_float();
	m_type = static_cast<Projectile::TYPE>(rd_int());
	m_parent = (Body*)rd_int();
}

void Projectile::PostLoadFixup()
{
	m_parent = Serializer::LookupBody((size_t)m_parent);
}

void Projectile::SetPosition(vector3d p)
{
	m_orient[12] = p.x;
	m_orient[13] = p.y;
	m_orient[14] = p.z;
}

void Projectile::NotifyDeleted(const Body* const deletedBody)
{
	if (m_parent == deletedBody) m_parent = 0;
}

void Projectile::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	SetPosition(GetPosition() + (m_baseVel+m_dirVel) * (double)timeStep);

	switch (m_type) {
		case TYPE_1MW_PULSE:
		case TYPE_2MW_PULSE:
		case TYPE_4MW_PULSE:
		case TYPE_10MW_PULSE:
		case TYPE_20MW_PULSE:
		case TYPE_17MW_MINING:
		case TYPE_SMALL_PLASMA_ACCEL:
		case TYPE_LARGE_PLASMA_ACCEL:
			if (m_age > PROJECTILE_AGE) Space::KillBody(this);
			break;
	}
}

/* In hull kg */
float Projectile::GetDamage() const
{
	float dam = 0;
	switch (m_type) {
		case TYPE_1MW_PULSE: dam = 1000.0f; break;
		case TYPE_2MW_PULSE: dam = 2000.0f; break;
		case TYPE_4MW_PULSE: dam = 4000.0f; break;
		case TYPE_10MW_PULSE: dam = 10000.0f; break;
		case TYPE_20MW_PULSE: dam = 20000.0f; break;
		case TYPE_17MW_MINING: dam = 17000.0f; break;
		case TYPE_SMALL_PLASMA_ACCEL: dam = 50000.0f; break;
		case TYPE_LARGE_PLASMA_ACCEL: dam = 100000.0f; break;
	}
	return dam * (PROJECTILE_AGE - m_age)/PROJECTILE_AGE;
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
	Space::AddBody(cargo);
}

void Projectile::StaticUpdate(const float timeStep)
{
	CollisionContact c;
	vector3d vel = m_baseVel + m_dirVel;
	GetFrame()->GetCollisionSpace()->TraceRay(GetPosition(), vel.Normalized(), vel.Length(), &c, 0);
	
	if (c.userData1) {
		Object *o = (Object*)c.userData1;

		if (o->IsType(Object::CITYONPLANET)) {
			Space::KillBody(this);
		}
		else if (o->IsType(Object::BODY)) {
			Body *hit = static_cast<Body*>(o);
			if (hit != m_parent) {
				hit->OnDamage(m_parent, GetDamage());
				Space::KillBody(this);
			}
		}
	}
	if (m_type == TYPE_17MW_MINING) {
		// need to test for terrain hit
		if (GetFrame()->m_astroBody && GetFrame()->m_astroBody->IsType(Object::PLANET)) {
			Planet *const planet = static_cast<Planet*>(GetFrame()->m_astroBody);
			const SBody *b = planet->GetSBody();
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				// hit the fucker
				if ((b->type == SBody::TYPE_PLANET_ASTEROID) ||
				    (b->type == SBody::TYPE_PLANET_LARGE_ASTEROID)) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), b, n*terrainHeight + 5.0*n);
					Sfx::Add(this, Sfx::TYPE_EXPLOSION);
				}
				Space::KillBody(this);
			}
		}
	}
}

void Projectile::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	static GLuint tex;
	if (!tex) tex = util_load_tex_rgba("data/textures/laser.png");

	vector3d from = viewTransform * GetPosition();
	vector3d to = viewTransform * (GetPosition() + 0.1*m_dirVel);
	vector3d dir = to - from;
		
	vector3f _from(&from.x);
	vector3f _dir(&dir.x);
	vector3f points[50];
	float p = 0;
	for (int i=0; i<50; i++, p+=0.02) {
		points[i] = _from + p*_dir;
	}
	Color col;

	Shader::EnableVertexProgram(Shader::VPROG_POINTSPRITE);

	switch (m_type) {
		case TYPE_1MW_PULSE:
			col = Color(1.0f, 0.0f, 0.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_2MW_PULSE:
			col = Color(1.0f, 0.5f, 0.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_4MW_PULSE:
			col = Color(1.0f, 1.0f, 0.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_10MW_PULSE:
			col = Color(0.0f, 1.0f, 0.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_20MW_PULSE:
			col = Color(0.0f, 0.0f, 1.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_17MW_MINING:
			col = Color(0.0f, 0.3f, 1.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 10.0f, col, tex);
			break;
		case TYPE_SMALL_PLASMA_ACCEL:
			col = Color(0.0f, 1.0f, 1.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 14.0f, col, tex);
			break;
		case TYPE_LARGE_PLASMA_ACCEL:
			col = Color(0.5f, 1.0f, 1.0f, 1.0f-(m_age/PROJECTILE_AGE));
			Render::PutPointSprites(50, points, 18.0f, col, tex);
			break;
	}

	Shader::DisableVertexProgram();
}

void Projectile::Add(Body *parent, TYPE t, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel)
{
	Projectile *p = new Projectile();
	p->m_parent = parent;
	p->m_type = t;
	p->SetFrame(parent->GetFrame());
	
	parent->GetRotMatrix(p->m_orient);
	p->SetPosition(pos);
	p->m_baseVel = baseVel;
	p->m_dirVel = dirVel;
	Space::AddBody(p);
}
