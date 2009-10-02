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
	}
	return dam * (PROJECTILE_AGE - m_age)/PROJECTILE_AGE;
}

void Projectile::StaticUpdate(const float timeStep)
{
	CollisionContact c;
	vector3d vel = m_baseVel + m_dirVel;
	GetFrame()->GetCollisionSpace()->TraceRay(GetPosition(), vel.Normalized(), vel.Length(), &c, 0);
	
	if (!c.userData1) return;

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

void Projectile::Render(const Frame *camFrame)
{
	static GLuint tex;
	if (!tex) tex = util_load_tex_rgba("data/textures/laser.png");

	matrix4x4d ftran;
	Frame::GetFrameTransform(GetFrame(), camFrame, ftran);
			
	vector3d from = ftran * GetPosition();
	vector3d to = ftran * (GetPosition() + 0.1*m_dirVel);
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
