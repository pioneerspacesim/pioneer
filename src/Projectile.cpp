#include "libs.h"
#include "Pi.h"
#include "Projectile.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "CargoBody.h"
#include "Planet.h"
#include "Sfx.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"

static const std::string projectileTextureFilename(PIONEER_DATA_DIR"/textures/projectile_l.png");
static const std::string projectileGlowTextureFilename(PIONEER_DATA_DIR"/textures/projectile_w.png");

Projectile::Projectile(): Body()
{
	m_orient = matrix4x4d::Identity();
	m_type = 1;
	m_age = 0;
	m_parent = 0;
	m_radius = 0;
	m_flags |= FLAG_DRAW_LAST;

	//set up materials
	m_sideMat.texture0 = Graphics::TextureBuilder::Billboard(projectileTextureFilename).GetOrCreateTexture(Pi::renderer, "billboard");
	m_sideMat.unlit = true;
	m_sideMat.twoSided = true;
	m_glowMat.texture0 = Graphics::TextureBuilder::Billboard(projectileGlowTextureFilename).GetOrCreateTexture(Pi::renderer, "billboard");
	m_glowMat.unlit = true;
	m_glowMat.twoSided = true;

	//zero at projectile position
	//+x down
	//+y right
	//+z forwards (or projectile direction)
	const float w = 0.5f;

	vector3f one(0.f, -w, 0.f); //top left
	vector3f two(0.f,  w, 0.f); //top right
	vector3f three(0.f,  w, -1.f); //bottom right
	vector3f four(0.f, -w, -1.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	m_sideVerts.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));
	m_glowVerts.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));

	//add four intersecting planes to create a volumetric effect
	for (int i=0; i < 4; i++) {
		m_sideVerts->Add(one, topLeft);
		m_sideVerts->Add(two, topRight);
		m_sideVerts->Add(three, botRight);

		m_sideVerts->Add(three, botRight);
		m_sideVerts->Add(four, botLeft);
		m_sideVerts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create quads for viewing on end
	float gw = 0.5f;
	float gz = -0.1f;

	for (int i=0; i < 4; i++) {
		m_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);
		m_glowVerts->Add(vector3f(-gw, gw, gz), topRight);
		m_glowVerts->Add(vector3f(gw, gw, gz), botRight);

		m_glowVerts->Add(vector3f(gw, gw, gz), botRight);
		m_glowVerts->Add(vector3f(gw, -gw, gz), botLeft);
		m_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);

		gw -= 0.1f; // they get smaller
		gz -= 0.2; // as they move back
	}
}

Projectile::~Projectile()
{
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
	m_radius = GetRadius();
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

double Projectile::GetRadius() const
{
	float length = Equip::lasers[m_type].length;
	float width = Equip::lasers[m_type].width;
	return sqrt(length*length + width*width);
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

void Projectile::Render(Graphics::Renderer *renderer, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	vector3d _from = viewTransform * GetInterpolatedPosition();
	vector3d _to = viewTransform * (GetInterpolatedPosition() + m_dirVel);
	vector3d _dir = _to - _from;
	vector3f from(&_from.x);
	vector3f dir = vector3f(_dir).Normalized();

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

	renderer->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
	renderer->SetDepthWrite(false);

	glPushMatrix();
	glMultMatrixf(&m[0]);

	// increase visible size based on distance from camera, z is always negative
	// allows them to be smaller while maintaining visibility for game play
	const float dist_scale = float(viewCoords.z / -500);
	const float length = Equip::lasers[m_type].length + dist_scale;
	const float width = Equip::lasers[m_type].width + dist_scale;
	glScalef(width, width, length);

	Color color = Equip::lasers[m_type].color;
	// fade them out as they age so they don't suddenly disappear
	// this matches the damage fall-off calculation
	const float base_alpha = sqrt(1.0f - m_age/Equip::lasers[m_type].lifespan);
	// fade out side quads when viewing nearly edge on
	vector3f view_dir = vector3f(viewCoords).Normalized();
	color.a = base_alpha * (1.f - powf(fabs(dir.Dot(view_dir)), length));

	if (color.a > 0.01f) {
		m_sideMat.diffuse = color;
		renderer->DrawTriangles(m_sideVerts.Get(), &m_sideMat);
	}

	// fade out glow quads when viewing nearly edge on
	// these and the side quads fade at different rates
	// so that they aren't both at the same alpha as that looks strange
	color.a = base_alpha * powf(fabs(dir.Dot(view_dir)), width);

	if (color.a > 0.01f) {
		m_glowMat.diffuse = color;
		renderer->DrawTriangles(m_glowVerts.Get(), &m_glowMat);
	}

	glPopMatrix();
	renderer->SetBlendMode(Graphics::BLEND_SOLID);
	renderer->SetDepthWrite(true);
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
	p->m_radius = p->GetRadius();
	Pi::game->GetSpace()->AddBody(p);
}
