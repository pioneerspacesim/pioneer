// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "Projectile.h"
#include "Frame.h"
#include "galaxy/StarSystem.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "CargoBody.h"
#include "Planet.h"
#include "Sfx.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"
#include "LuaEvent.h"
#include "LuaUtils.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"

std::unique_ptr<Graphics::VertexArray> Projectile::s_sideVerts;
std::unique_ptr<Graphics::VertexArray> Projectile::s_glowVerts;
std::unique_ptr<Graphics::Material> Projectile::s_sideMat;
std::unique_ptr<Graphics::Material> Projectile::s_glowMat;
Graphics::RenderState *Projectile::s_renderState = nullptr;

void Projectile::BuildModel()
{
	//set up materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	s_sideMat.reset(Pi::renderer->CreateMaterial(desc));
	s_glowMat.reset(Pi::renderer->CreateMaterial(desc));
	s_sideMat->texture0 = Graphics::TextureBuilder::Billboard("textures/projectile_l.png").GetOrCreateTexture(Pi::renderer, "billboard");
	s_glowMat->texture0 = Graphics::TextureBuilder::Billboard("textures/projectile_w.png").GetOrCreateTexture(Pi::renderer, "billboard");

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

	s_sideVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));
	s_glowVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0));

	//add four intersecting planes to create a volumetric effect
	for (int i=0; i < 4; i++) {
		s_sideVerts->Add(one, topLeft);
		s_sideVerts->Add(two, topRight);
		s_sideVerts->Add(three, botRight);

		s_sideVerts->Add(three, botRight);
		s_sideVerts->Add(four, botLeft);
		s_sideVerts->Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create quads for viewing on end
	float gw = 0.5f;
	float gz = -0.1f;

	for (int i=0; i < 4; i++) {
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);
		s_glowVerts->Add(vector3f(-gw, gw, gz), topRight);
		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);

		s_glowVerts->Add(vector3f(gw, gw, gz), botRight);
		s_glowVerts->Add(vector3f(gw, -gw, gz), botLeft);
		s_glowVerts->Add(vector3f(-gw, -gw, gz), topLeft);

		gw -= 0.1f; // they get smaller
		gz -= 0.2f; // as they move back
	}

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.cullMode = Graphics::CULL_NONE;
	s_renderState = Pi::renderer->CreateRenderState(rsd);
}

void Projectile::FreeModel()
{
	s_sideMat.reset();
	s_glowMat.reset();
	s_sideVerts.reset();
	s_glowVerts.reset();
}

Projectile::Projectile(): Body()
{
	if (!s_sideMat) BuildModel();
	SetOrient(matrix3x3d::Identity());
	m_lifespan = 0;
	m_baseDam = 0;
	m_length = 0;
	m_width = 0;
	m_mining = false;
	m_age = 0;
	m_parent = 0;
	m_flags |= FLAG_DRAW_LAST;
}

Projectile::~Projectile()
{
}

void Projectile::Save(Serializer::Writer &wr, Space *space)
{
	Body::Save(wr, space);
	wr.Vector3d(m_baseVel);
	wr.Vector3d(m_dirVel);
	wr.Float(m_age);
	wr.Float(m_lifespan);
	wr.Float(m_baseDam);
	wr.Float(m_length);
	wr.Float(m_width);
	wr.Bool(m_mining);
	wr.Color4UB(m_color);
	wr.Int32(space->GetIndexForBody(m_parent));
}

void Projectile::Load(Serializer::Reader &rd, Space *space)
{
	Body::Load(rd, space);
	m_baseVel = rd.Vector3d();
	m_dirVel = rd.Vector3d();
	m_age = rd.Float();
	m_lifespan = rd.Float();
	m_baseDam = rd.Float();
	m_length = rd.Float();
	m_width = rd.Float();
	m_mining = rd.Bool();
	m_color = rd.Color4UB();
	m_parentIndex = rd.Int32();
}

void Projectile::PostLoadFixup(Space *space)
{
	Body::PostLoadFixup(space);
	m_parent = space->GetBodyByIndex(m_parentIndex);
}

void Projectile::UpdateInterpTransform(double alpha)
{
	m_interpOrient = GetOrient();
	const vector3d oldPos = GetPosition() - (m_baseVel+m_dirVel)*Pi::game->GetTimeStep();
	m_interpPos = alpha*GetPosition() + (1.0-alpha)*oldPos;
}

void Projectile::NotifyRemoved(const Body* const removedBody)
{
	if (m_parent == removedBody) m_parent = 0;
}

void Projectile::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	SetPosition(GetPosition() + (m_baseVel+m_dirVel) * double(timeStep));
	if (m_age > m_lifespan) Pi::game->GetSpace()->KillBody(this);
}

/* In hull kg */
float Projectile::GetDamage() const
{
	return m_baseDam * sqrt((m_lifespan - m_age)/m_lifespan);
	// TEST
//	return 0.01f;
}

double Projectile::GetRadius() const
{
	return sqrt(m_length*m_length + m_width*m_width);
}

static void MiningLaserSpawnTastyStuff(Frame *f, const SystemBody *asteroid, const vector3d &pos)
{
	lua_State *l = Lua::manager->GetLuaState();
	pi_lua_import(l, "Equipment");
	LuaTable cargo_types = LuaTable(l, -1).Sub("cargo");
	if (20*Pi::rng.Fixed() < asteroid->GetMetallicity()) {
		cargo_types.Sub("precious_metals");
	} else if (8*Pi::rng.Fixed() < asteroid->GetMetallicity()) {
		cargo_types.Sub("metal_alloys");
	} else if (Pi::rng.Fixed() < asteroid->GetMetallicity()) {
		cargo_types.Sub("metal_ore");
	} else if (Pi::rng.Fixed() < fixed(1,2)) {
		cargo_types.Sub("water");
	} else {
		cargo_types.Sub("rubbish");
	}
	CargoBody *cargo = new CargoBody(LuaRef(l, -1));
	lua_pop(l, 3);
	cargo->SetFrame(f);
	cargo->SetPosition(pos);
	const double x = Pi::rng.Double();
	vector3d dir = pos.Normalized();
	dir.ArbRotate(vector3d(x, 1-x, 0), Pi::rng.Double()-.5);
	cargo->SetVelocity(Pi::rng.Double(100.0,200.0) * dir);
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
				hit->OnDamage(m_parent, GetDamage(), c);
				Pi::game->GetSpace()->KillBody(this);
				if (hit->IsType(Object::SHIP))
					LuaEvent::Queue("onShipHit", dynamic_cast<Ship*>(hit), dynamic_cast<Body*>(m_parent));
			}
		}
	}
	if (m_mining) {
		// need to test for terrain hit
		if (GetFrame()->GetBody() && GetFrame()->GetBody()->IsType(Object::PLANET)) {
			Planet *const planet = static_cast<Planet*>(GetFrame()->GetBody());
			const SystemBody *b = planet->GetSystemBody();
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				// hit the fucker
				if (b->GetType() == SystemBody::TYPE_PLANET_ASTEROID) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), b, n*terrainHeight + 5.0*n);
					Sfx::Add(this, Sfx::TYPE_EXPLOSION);
				}
				Pi::game->GetSpace()->KillBody(this);
			}
		}
	}
}

void Projectile::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	vector3d _from = viewTransform * GetInterpPosition();
	vector3d _to = viewTransform * (GetInterpPosition() + m_dirVel);
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

	// increase visible size based on distance from camera, z is always negative
	// allows them to be smaller while maintaining visibility for game play
	const float dist_scale = float(viewCoords.z / -500);
	const float length = m_length + dist_scale;
	const float width = m_width + dist_scale;

	renderer->SetTransform(m * matrix4x4f::ScaleMatrix(width, width, length));

	Color color = m_color;
	// fade them out as they age so they don't suddenly disappear
	// this matches the damage fall-off calculation
	const float base_alpha = sqrt(1.0f - m_age/m_lifespan);
	// fade out side quads when viewing nearly edge on
	vector3f view_dir = vector3f(viewCoords).Normalized();
	color.a = (base_alpha * (1.f - powf(fabs(dir.Dot(view_dir)), length))) * 255;

	if (color.a > 3) {
		s_sideMat->diffuse = color;
		renderer->DrawTriangles(s_sideVerts.get(), s_renderState, s_sideMat.get());
	}

	// fade out glow quads when viewing nearly edge on
	// these and the side quads fade at different rates
	// so that they aren't both at the same alpha as that looks strange
	color.a = (base_alpha * powf(fabs(dir.Dot(view_dir)), width)) * 255;

	if (color.a > 3) {
		s_glowMat->diffuse = color;
		renderer->DrawTriangles(s_glowVerts.get(), s_renderState, s_glowMat.get());
	}
}

void Projectile::Add(Body *parent, float lifespan, float dam, float length, float width, bool mining, const Color &color, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel)
{
	Projectile *p = new Projectile();
	p->m_parent = parent;
	p->m_lifespan = lifespan;
	p->m_baseDam = dam;
	p->m_length = length;
	p->m_width = width;
	p->m_mining = mining;
	p->m_color = color;
	p->SetFrame(parent->GetFrame());

	p->SetOrient(parent->GetOrient());
	p->SetPosition(pos);
	p->m_baseVel = baseVel;
	p->m_dirVel = dirVel;
	p->SetClipRadius(p->GetRadius());
	p->SetPhysRadius(p->GetRadius());
	Pi::game->GetSpace()->AddBody(p);
}
