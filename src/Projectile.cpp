// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Projectile.h"

#include "CargoBody.h"
#include "Frame.h"
#include "Game.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Sfx.h"
#include "Ship.h"
#include "Space.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "lua/LuaEvent.h"
#include "lua/LuaUtils.h"

std::unique_ptr<Graphics::MeshObject> Projectile::s_sideMesh;
std::unique_ptr<Graphics::MeshObject> Projectile::s_glowMesh;
std::unique_ptr<Graphics::Material> Projectile::s_sideMat;
std::unique_ptr<Graphics::Material> Projectile::s_glowMat;

void Projectile::BuildModel()
{
	//set up materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.cullMode = Graphics::CULL_NONE;

	s_sideMat.reset(Pi::renderer->CreateMaterial("unlit", desc, rsd));
	s_sideMat->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard("textures/projectile_l.dds").GetOrCreateTexture(Pi::renderer, "billboard"));
	s_glowMat.reset(Pi::renderer->CreateMaterial("unlit", desc, rsd));
	s_glowMat->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard("textures/projectile_w.dds").GetOrCreateTexture(Pi::renderer, "billboard"));

	//zero at projectile position
	//+x down
	//+y right
	//+z forwards (or projectile direction)
	const float w = 0.5f;

	vector3f one(0.f, -w, 0.f);	  //top left
	vector3f two(0.f, w, 0.f);	  //top right
	vector3f three(0.f, w, -1.f); //bottom right
	vector3f four(0.f, -w, -1.f); //bottom left

	//uv coords
	const vector2f topLeft(0.f, 1.f);
	const vector2f topRight(1.f, 1.f);
	const vector2f botLeft(0.f, 0.f);
	const vector2f botRight(1.f, 0.f);

	Graphics::VertexArray sideVerts(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	Graphics::VertexArray glowVerts(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	//add four intersecting planes to create a volumetric effect
	for (int i = 0; i < 4; i++) {
		sideVerts.Add(one, topLeft);
		sideVerts.Add(two, topRight);
		sideVerts.Add(three, botRight);

		sideVerts.Add(three, botRight);
		sideVerts.Add(four, botLeft);
		sideVerts.Add(one, topLeft);

		one.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		two.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		three.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
		four.ArbRotate(vector3f(0.f, 0.f, 1.f), DEG2RAD(45.f));
	}

	//create quads for viewing on end
	float gw = 0.5f;
	float gz = -0.1f;

	for (int i = 0; i < 4; i++) {
		glowVerts.Add(vector3f(-gw, -gw, gz), topLeft);
		glowVerts.Add(vector3f(-gw, gw, gz), topRight);
		glowVerts.Add(vector3f(gw, gw, gz), botRight);

		glowVerts.Add(vector3f(gw, gw, gz), botRight);
		glowVerts.Add(vector3f(gw, -gw, gz), botLeft);
		glowVerts.Add(vector3f(-gw, -gw, gz), topLeft);

		gw -= 0.1f; // they get smaller
		gz -= 0.2f; // as they move back
	}

	s_sideMesh.reset(Pi::renderer->CreateMeshObjectFromArray(&sideVerts));
	s_glowMesh.reset(Pi::renderer->CreateMeshObjectFromArray(&glowVerts));
}

void Projectile::FreeModel()
{
	s_sideMat.reset();
	s_glowMat.reset();
	s_sideMesh.reset();
	s_glowMesh.reset();
}

Projectile::Projectile(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel) :
	Body()
{
	if (!s_sideMat) BuildModel();
	m_flags |= FLAG_DRAW_LAST;

	m_parent = parent;
	m_lifespan = prData.lifespan;
	m_baseDam = prData.damage;
	m_length = prData.length;
	m_width = prData.width;
	m_mining = prData.mining;
	m_color = prData.color;
	m_age = 0;

	SetFrame(parent->GetFrame());

	SetOrient(parent->GetOrient());
	SetPosition(pos);
	m_baseVel = baseVel;
	m_dirVel = dirVel;
	SetClipRadius(GetRadius());
	SetPhysRadius(GetRadius());
}

Projectile::Projectile(const Json &jsonObj, Space *space) :
	Body(jsonObj, space)
{
	if (!s_sideMat) BuildModel();

	try {
		Json projectileObj = jsonObj["projectile"];

		m_baseVel = projectileObj["base_vel"];
		m_dirVel = projectileObj["dir_vel"];
		m_age = projectileObj["age"];
		m_lifespan = projectileObj["life_span"];
		m_baseDam = projectileObj["base_dam"];
		m_length = projectileObj["length"];
		m_width = projectileObj["width"];
		m_mining = projectileObj["mining"];
		m_color = projectileObj["color"];
		m_parentIndex = projectileObj["index_for_body"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

Projectile::~Projectile()
{
}

void Projectile::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json projectileObj({}); // Create JSON object to contain projectile data.

	projectileObj["base_vel"] = m_baseVel;
	projectileObj["dir_vel"] = m_dirVel;
	projectileObj["age"] = m_age;
	projectileObj["life_span"] = m_lifespan;
	projectileObj["base_dam"] = m_baseDam;
	projectileObj["length"] = m_length;
	projectileObj["width"] = m_width;
	projectileObj["mining"] = m_mining;
	projectileObj["color"] = m_color;
	projectileObj["index_for_body"] = space->GetIndexForBody(m_parent);

	jsonObj["projectile"] = projectileObj; // Add projectile object to supplied object.
}

void Projectile::PostLoadFixup(Space *space)
{
	Body::PostLoadFixup(space);
	m_parent = space->GetBodyByIndex(m_parentIndex);
}

void Projectile::UpdateInterpTransform(double alpha)
{
	m_interpOrient = GetOrient();
	const vector3d oldPos = GetPosition() - (m_baseVel + m_dirVel) * Pi::game->GetTimeStep();
	m_interpPos = alpha * GetPosition() + (1.0 - alpha) * oldPos;
}

void Projectile::NotifyRemoved(const Body *const removedBody)
{
	if (m_parent == removedBody) m_parent = 0;
}

void Projectile::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	SetPosition(GetPosition() + (m_baseVel + m_dirVel) * double(timeStep));
	if (m_age > m_lifespan) Pi::game->GetSpace()->KillBody(this);
}

/* In hull kg */
float Projectile::GetDamage() const
{
	return m_baseDam * sqrt((m_lifespan - m_age) / m_lifespan);
	// TEST
	//	return 0.01f;
}

double Projectile::GetRadius() const
{
	return sqrt(m_length * m_length + m_width * m_width);
}

void MiningLaserSpawnTastyStuff(FrameId fId, const SystemBody *asteroid, const vector3d &pos)
{
	lua_State *l = Lua::manager->GetLuaState();

	// lua cant push "const SystemBody", needs to convert to non-const
	RefCountedPtr<StarSystem> s = Pi::game->GetGalaxy()->GetStarSystem(asteroid->GetPath());
	SystemBody *liveasteroid = s->GetBodyByPath(asteroid->GetPath());

	// this is an adapted version of "CallMethod", because;
	// 1, there is no template for LuaObject<LuaTable>::CallMethod(..., SystemBody)
	// 2, this leaves the return value on the lua stack to be used by "new CargoBody()"
	LUA_DEBUG_START(l);
	LuaObject<Player>::PushToLua(Pi::player);
	lua_pushstring(l, "SpawnMiningContainer");
	lua_gettable(l, -2);
	lua_pushvalue(l, -2);
	lua_remove(l, -3);
	LuaObject<SystemBody>::PushToLua(liveasteroid);
	pi_lua_protected_call(l, 2, 1);

	CargoBody *cargo = new CargoBody(LuaRef(l, -1));
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);

	cargo->SetFrame(fId);
	cargo->SetPosition(pos);
	const double x = Pi::rng.Double();
	vector3d dir = pos.Normalized();
	dir.ArbRotate(vector3d(x, 1 - x, 0), Pi::rng.Double() - .5);
	cargo->SetVelocity(Pi::rng.Double(100.0, 200.0) * dir);
	Pi::game->GetSpace()->AddBody(cargo);
}

void Projectile::StaticUpdate(const float timeStep)
{
	PROFILE_SCOPED()
	CollisionContact c;
	// Collision spaces don't store velocity, so dirvel-only is still wrong but less awful than dirvel+basevel
	vector3d vel = m_dirVel * timeStep;
	Frame *frame = Frame::GetFrame(GetFrame());
	frame->GetCollisionSpace()->TraceRay(GetPosition(), vel.Normalized(), vel.Length(), &c);

	if (c.userData1) {
		Body *hit = static_cast<Body *>(c.userData1);
		if (hit != m_parent) {
			hit->OnDamage(m_parent, GetDamage(), c);
			Pi::game->GetSpace()->KillBody(this);
			if (hit->IsType(ObjectType::SHIP))
				LuaEvent::Queue("onShipHit", dynamic_cast<Ship *>(hit), dynamic_cast<Body *>(m_parent));
		}
	}
	if (m_mining) // mining lasers can break off chunks of terrain
	{
		// need to test for terrain hit
		Planet *const planet = static_cast<Planet *>(frame->GetBody()); // cache the value even for the if statement
		if (planet && planet->IsType(ObjectType::PLANET)) {
			vector3d pos = GetPosition();
			double terrainHeight = planet->GetTerrainHeight(pos.Normalized());
			if (terrainHeight > pos.Length()) {
				const SystemBody *b = planet->GetSystemBody();
				// hit the fucker
				if (b->GetType() == SystemBody::TYPE_PLANET_ASTEROID) {
					vector3d n = GetPosition().Normalized();
					MiningLaserSpawnTastyStuff(planet->GetFrame(), b, n * terrainHeight + 5.0 * n);
					SfxManager::Add(this, TYPE_EXPLOSION);
				}
				Pi::game->GetSpace()->KillBody(this);
			}
		}
	}
}

void Projectile::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	PROFILE_SCOPED()
	vector3d _from = viewTransform * GetInterpPosition();
	vector3d _to = viewTransform * (GetInterpPosition() + m_dirVel);
	vector3d _dir = _to - _from;
	vector3f from(&_from.x);
	vector3f dir = vector3f(_dir).Normalized();

	vector3f v1, v2;
	matrix4x4f m = matrix4x4f::Identity();
	v1.x = dir.y;
	v1.y = dir.z;
	v1.z = dir.x;
	v2 = v1.Cross(dir).Normalized();
	v1 = v2.Cross(dir);
	m[0] = v1.x;
	m[4] = v2.x;
	m[8] = dir.x;
	m[1] = v1.y;
	m[5] = v2.y;
	m[9] = dir.y;
	m[2] = v1.z;
	m[6] = v2.z;
	m[10] = dir.z;

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
	const float base_alpha = sqrt(1.0f - m_age / m_lifespan);
	// fade out side quads when viewing nearly edge on
	vector3f view_dir = vector3f(viewCoords).Normalized();
	color.a = (base_alpha * (1.f - powf(fabs(dir.Dot(view_dir)), length))) * 255;

	if (color.a > 3) {
		s_sideMat->diffuse = color;
		renderer->DrawMesh(s_sideMesh.get(), s_sideMat.get());
	}

	// fade out glow quads when viewing nearly edge on
	// these and the side quads fade at different rates
	// so that they aren't both at the same alpha as that looks strange
	color.a = (base_alpha * powf(fabs(dir.Dot(view_dir)), width)) * 255;

	if (color.a > 3) {
		s_glowMat->diffuse = color;
		renderer->DrawMesh(s_glowMesh.get(), s_glowMat.get());
	}
}

void Projectile::Add(Body *parent, float lifespan, float dam, float length, float width, bool mining, const Color &color, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel)
{
	ProjectileData prData;
	prData.lifespan = lifespan;
	prData.damage = dam;
	prData.length = length;
	prData.width = width;
	prData.mining = mining;
	prData.color = color;
	Projectile *p = new Projectile(parent, prData, pos, baseVel, dirVel);
	Pi::game->GetSpace()->AddBody(p);
}

void Projectile::Add(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel)
{
	Projectile *p = new Projectile(parent, prData, pos, baseVel, dirVel);
	Pi::game->GetSpace()->AddBody(p);
}
