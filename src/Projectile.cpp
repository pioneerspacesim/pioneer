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

	m_prog = new Render::Shader("flat", "#define TEXTURE0 1\n");

	m_sideTex = Pi::textureCache->GetBillboardTexture(PIONEER_DATA_DIR "/textures/laser.png");
	m_glowTex = Pi::textureCache->GetBillboardTexture(PIONEER_DATA_DIR "/textures/halo.png");

	//zero at projectile position
	//+x down
	//+y right
	//+z forwards (or projectile direction)
	const float w = 0.25f;

	vector3f one(0.f, -w, 0.f); //top left
	vector3f two(0.f,  w, 0.f); //top right
	vector3f three(0.f,  w, -1.f); //bottom right
	vector3f four(0.f, -w, -1.f); //bottom left

	//add four intersecting planes to create a volumetric effect
	for (int i=0; i < 4; i++) {
		m_verts.push_back(Vertex(one, 0.f, 1.f));
		m_verts.push_back(Vertex(two, 1.f, 1.f));
		m_verts.push_back(Vertex(three, 1.f, 0.f));

		m_verts.push_back(Vertex(three, 1.f, 0.f));
		m_verts.push_back(Vertex(four, 0.f, 0.f));
		m_verts.push_back(Vertex(one, 0.f, 1.f));

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create quads for viewing on end
	//these are added in the same vertex array to avoid a
	//vertex pointer change
	float gw = 0.25f;
	float gz = -0.1f;

	for (int i=0; i < 3; i++) {
		m_verts.push_back(Vertex(vector3f(-gw, -gw, gz), 0.f, 1.f));
		m_verts.push_back(Vertex(vector3f(-gw, gw, gz), 1.f, 1.f));
		m_verts.push_back(Vertex(vector3f(gw, gw, gz),1.f, 0.f));

		m_verts.push_back(Vertex(vector3f(gw, gw, gz), 1.f, 0.f));
		m_verts.push_back(Vertex(vector3f(gw, -gw, gz), 0.f, 0.f));
		m_verts.push_back(Vertex(vector3f(-gw, -gw, gz), 0.f, 1.f));

		gw -= 0.05f; // they get smaller
		gz -= 0.3; // as they move back
	}
}

Projectile::~Projectile()
{
	delete m_prog;
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
	vector3d vel = (m_baseVel+m_dirVel) * timeStep;
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
	vector3d _from = viewTransform * GetInterpolatedPosition();
	vector3d _to = viewTransform * (GetInterpolatedPosition() + m_dirVel);
	vector3d _dir = _to - _from;
	vector3f from(&_from.x);
	vector3f dir = vector3f(_dir).Normalized();

	Color color = Equip::lasers[m_type].color;
	float base_alpha = sqrt(1.0f - m_age/Equip::lasers[m_type].lifespan);
	float size = Equip::lasers[m_type].psize * base_alpha;

	vector3f v1, v2;
	matrix4x4f m = matrix4x4f::Identity();
	v1.x = dir.y; v1.y = dir.z; v1.z = dir.x;
	v2 = v1.Cross(dir).Normalized();
	v1 = v2.Cross(dir);
	m[0] = v1.x; m[4] = v2.x; m[8] = dir.x;
	m[1] = v1.y; m[5] = v2.y; m[9] = dir.y;
	m[2] = v1.z; m[6] = v2.z; m[10] = dir.z;

	m[12] = from.x;
	m[13] = from.y;
	m[14] = from.z;

	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix ();
	glMultMatrixf (&m[0]);

	glScalef (size, size, size*3);

	//fade out side quads when facing nearly edge on
	vector3f cdir(0.f, 0.f, 1.f);
	color.a = base_alpha * (1.f - powf(fabs(dir.Dot(cdir)), size*size));

	m_sideTex->Bind();
	Render::State::UseProgram(m_prog);
	m_prog->SetUniform("texture0", 0);
	m_prog->SetUniform("color", color);
	glColor4f(color.r, color.g, color.b, color.a);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_verts[0].pos);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m_verts[0].u);
	const int flare_size = 4*6;
	glDrawArrays(GL_TRIANGLES, 0, flare_size);

	//fade out glow quads when facing nearly edge on
	color.a = base_alpha * powf(fabs(dir.Dot(cdir)), size*0.5f);

	m_glowTex->Bind();
	m_prog->SetUniform("color", color);
	glColor4f(color.r, color.g, color.b, color.a);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_verts[0].pos);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m_verts[0].u);
	glDrawArrays(GL_TRIANGLES, flare_size, 3*6);
	m_glowTex->Unbind();

	glPopMatrix ();
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.f, 1.f, 1.f);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
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
