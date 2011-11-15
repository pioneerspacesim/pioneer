#include "libs.h"
#include "Space.h"
#include "Body.h"
#include "Frame.h"
#include "Star.h"
#include "Planet.h"
#include <algorithm>
#include <functional>
#include "Pi.h"
#include "Player.h"
#include "StarSystem.h"
#include "SpaceStation.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "Missile.h"
#include "HyperspaceCloud.h"
#include "render/Render.h"
#include "WorldView.h"
#include "SectorView.h"
#include "Lang.h"
#include "ShipCpanel.h"

namespace Space {

std::list<Body*> bodies;
Frame *rootFrame;
static void CollideFrame(Frame *f);
static void PruneCorpses();
static std::list<Body*> corpses;
static SystemPath *hyperspacingTo;
static float hyperspaceAnim;
static double hyperspaceDuration;
static double hyperspaceEndTime;
static std::list<HyperspaceCloud*> storedArrivalClouds;

void Init()
{
	rootFrame = new Frame(NULL, Lang::SYSTEM);
	rootFrame->SetRadius(FLT_MAX);
}

void Uninit()
{
	delete rootFrame;
	Pi::currentSystem.Reset();
}

void Clear()
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->SetFrame(NULL);
		if ((*i) != static_cast<Body*>(Pi::player)) {
			KillBody(*i);
		}
	}
	PruneCorpses();

	Pi::player->SetFrame(rootFrame);
	for (std::list<Frame*>::iterator i = rootFrame->m_children.begin(); i != rootFrame->m_children.end(); ++i) delete *i;
	rootFrame->m_children.clear();
	rootFrame->m_astroBody = 0;
	rootFrame->m_sbody = 0;

	if (hyperspacingTo) delete hyperspacingTo;
	hyperspacingTo = 0;
	hyperspaceAnim = 0.0f;
	hyperspaceDuration = 0.0f;
	hyperspaceEndTime = 0.0f;
}

Body *FindNearestTo(const Body *b, Object::Type t)
{
	Body *nearest = 0;
	double dist = FLT_MAX;
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->IsDead()) continue;
		if ((*i)->IsType(t)) {
			double d = (*i)->GetPositionRelTo(b).Length();
			if (d < dist) {
				dist = d;
				nearest = *i;
			}
		}
	}
	return nearest;
}

Body *FindBodyForPath(const SystemPath *path)
{
	// it is a bit dumb that currentSystem is not part of Space...
	SBody *body = Pi::currentSystem->GetBodyByPath(path);

	if (!body) return 0;

	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->GetSBody() == body) return *i;
	}
	return 0;
}

// XXX this is only called by Missile::Explode. consider moving it there
void RadiusDamage(Body *attacker, Frame *f, const vector3d &pos, double radius, double kgDamage)
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->GetFrame() != f) continue;
		double dist = ((*i)->GetPosition() - pos).Length();
		if (dist < radius) {
			// linear damage decay with distance
			(*i)->OnDamage(attacker, kgDamage * (radius - dist) / radius);
			if ((*i)->IsType(Object::SHIP))
				Pi::luaOnShipHit->Queue(dynamic_cast<Ship*>(*i), attacker);
		}
	}
}

void DoECM(const Frame *f, const vector3d &pos, int power_val)
{
	const float ECM_RADIUS = 4000.0f;
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->GetFrame() != f) continue;
		if (!(*i)->IsType(Object::MISSILE)) continue;

		double dist = ((*i)->GetPosition() - pos).Length();
		if (dist < ECM_RADIUS) {
			// increasing chance of destroying it with proximity
			if (Pi::rng.Double() > (dist / ECM_RADIUS)) {
				static_cast<Missile*>(*i)->ECMAttack(power_val);
			}
		}
	}

}

void Serialize(Serializer::Writer &wr)
{
	Serializer::Writer wr2;
	Frame::Serialize(wr2, rootFrame);
	wr.WrSection("Frames", wr2.GetData());

	wr.Int32(bodies.size());
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		//printf("Serializing %s\n", (*i)->GetLabel().c_str());
		(*i)->Serialize(wr);
	}
	wr.Int32(storedArrivalClouds.size());
	for (std::list<HyperspaceCloud*>::iterator i = storedArrivalClouds.begin();
			i != storedArrivalClouds.end(); ++i) {
		(*i)->Serialize(wr);
	}
	if (hyperspacingTo == 0) {
		wr.Byte(0);
	} else {
		wr.Byte(1);
		hyperspacingTo->Serialize(wr);
		wr.Float(hyperspaceAnim);
		wr.Double(hyperspaceDuration);
		wr.Double(hyperspaceEndTime);
	}
}

void Unserialize(Serializer::Reader &rd)
{
	Serializer::IndexSystemBodies(Pi::currentSystem.Get());
	
	Serializer::Reader rd2 = rd.RdSection("Frames");
	rootFrame = Frame::Unserialize(rd2, 0);
	
	// XXX not needed. done in Pi::Unserialize
	Serializer::IndexFrames();
	int num_bodies = rd.Int32();
	//printf("%d bodies to read\n", num_bodies);
	for (int i=0; i<num_bodies; i++) {
		Body *b = Body::Unserialize(rd);
		if (b) bodies.push_back(b);
	}
	num_bodies = rd.Int32();
	for (int i=0; i<num_bodies; i++) {
		Body *b = Body::Unserialize(rd);
		if (b) storedArrivalClouds.push_back(static_cast<HyperspaceCloud*>(b));
	}

	hyperspaceAnim = 0;
	if (rd.Byte()) {
		hyperspacingTo = new SystemPath(SystemPath::Unserialize(rd));
		hyperspaceAnim = rd.Float();
		hyperspaceDuration = rd.Double();
		hyperspaceEndTime = rd.Double();
	}
	// bodies with references to others must fix these up
	Serializer::IndexBodies();
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->PostLoadFixup();
	}
	Frame::PostUnserializeFixup(rootFrame);
}

static Frame *find_frame_with_sbody(Frame *f, const SBody *b)
{
	if (f->m_sbody == b) return f;
	else {
		for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
			
			Frame *found = find_frame_with_sbody(*i, b);
			if (found) return found;
		}
	}
	return 0;
}

Frame *GetFrameWithSBody(const SBody *b)
{
	return find_frame_with_sbody(rootFrame, b);
}

static void SetFrameOrientationFromSBodyAxialTilt(Frame *f, const SBody *sbody)
{
	matrix4x4d rot = matrix4x4d::RotateXMatrix(sbody->axialTilt.ToDouble());
	f->SetRotationOnly(rot);
}

static Frame *MakeFrameFor(SBody *sbody, Body *b, Frame *f)
{
	Frame *orbFrame, *rotFrame;
	double frameRadius;

	if (!sbody->parent) {
		if (b) b->SetFrame(f);
		f->m_sbody = sbody;
		f->m_astroBody = b;
		return f;
	}

	if (sbody->type == SBody::TYPE_GRAVPOINT) {
		orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->m_sbody = sbody;
		orbFrame->m_astroBody = b;
		orbFrame->SetRadius(sbody->GetMaxChildOrbitalDistance()*1.1);
		return orbFrame;
	}

	SBody::BodySuperType supertype = sbody->GetSuperType();

	if ((supertype == SBody::SUPERTYPE_GAS_GIANT) ||
	    (supertype == SBody::SUPERTYPE_ROCKY_PLANET)) {
		// for planets we want an non-rotating frame for a few radii
		// and a rotating frame in the same position but with maybe 1.05*radius,
		// which actually contains the object.
		frameRadius = std::max(4.0*sbody->GetRadius(), sbody->GetMaxChildOrbitalDistance()*1.05);
		orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->m_sbody = sbody;
		orbFrame->SetRadius(frameRadius);
		//printf("\t\t\t%s has frame size %.0fkm, body radius %.0fkm\n", sbody->name.c_str(),
		//	(frameRadius ? frameRadius : 10*sbody->GetRadius())*0.001f,
		//	sbody->GetRadius()*0.001f);
	
		assert(sbody->rotationPeriod != 0);
		rotFrame = new Frame(orbFrame, sbody->name.c_str());
		// rotating frame has size of GeoSphere terrain bounding sphere
		rotFrame->SetRadius(b->GetBoundingRadius());
		rotFrame->SetAngVelocity(vector3d(0,2*M_PI/sbody->GetRotationPeriod(),0));
		rotFrame->m_astroBody = b;
		SetFrameOrientationFromSBodyAxialTilt(rotFrame, sbody);
		b->SetFrame(rotFrame);
		return orbFrame;
	}
	else if (supertype == SBody::SUPERTYPE_STAR) {
		// stars want a single small non-rotating frame
		orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->m_sbody = sbody;
		orbFrame->m_astroBody = b;
		orbFrame->SetRadius(sbody->GetMaxChildOrbitalDistance()*1.1);
		b->SetFrame(orbFrame);
		return orbFrame;
	}
	else if (sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
		// space stations want non-rotating frame to some distance
		// and a much closer rotating frame
		frameRadius = 1000000.0; // XXX NFI!
		orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->m_sbody = sbody;
//		orbFrame->SetRadius(10*sbody->GetRadius());
		orbFrame->SetRadius(frameRadius);
	
		assert(sbody->rotationPeriod != 0);
		rotFrame = new Frame(orbFrame, sbody->name.c_str());
		rotFrame->SetRadius(1000.0);
//		rotFrame->SetRadius(1.1*sbody->GetRadius());		// enough for collisions?
		rotFrame->SetAngVelocity(vector3d(0.0,double(static_cast<SpaceStation*>(b)->GetDesiredAngVel()),0.0));
		rotFrame->m_astroBody = b;		// hope this doesn't break anything
		b->SetFrame(rotFrame);
		return orbFrame;
	} else if (sbody->type == SBody::TYPE_STARPORT_SURFACE) {
		// just put body into rotating frame of planet, not in its own frame
		// (because collisions only happen between objects in same frame,
		// and we want collisions on starport and on planet itself)
		Frame *frame = *f->m_children.begin();
		b->SetFrame(frame);
		assert(frame->m_astroBody->IsType(Object::PLANET));
		Planet *planet = static_cast<Planet*>(frame->m_astroBody);

		/* position on planet surface */
		double height;
		int tries;
		matrix4x4d rot;
		vector3d pos;
		// first try suggested position
		rot = sbody->orbit.rotMatrix;
		pos = rot * vector3d(0,1,0);
		if (planet->GetTerrainHeight(pos) - planet->GetSBody()->GetRadius() <= 0.0) {
			MTRand r(sbody->seed);
			// position is under water. try some random ones
			for (tries=0; tries<100; tries++) {
				// used for orientation on planet surface
				double r2 = r.Double(); 	// function parameter evaluation order is implementation-dependent
				double r1 = r.Double();		// can't put two rands in the same expression
				rot = matrix4x4d::RotateZMatrix(2*M_PI*r1)
					* matrix4x4d::RotateYMatrix(2*M_PI*r2);
				pos = rot * vector3d(0,1,0);
				height = planet->GetTerrainHeight(pos) - planet->GetSBody()->GetRadius();
				// don't want to be under water
				if (height > 0.0) break;
			}
		}
		b->SetPosition(pos * planet->GetTerrainHeight(pos));
		b->SetRotMatrix(rot);
		return frame;
	} else {
		assert(0);
	}
	return NULL;
}

void GenBody(SBody *sbody, Frame *f)
{
	Body *b = 0;

	if (sbody->type != SBody::TYPE_GRAVPOINT) {
		if (sbody->GetSuperType() == SBody::SUPERTYPE_STAR) {
			Star *star = new Star(sbody);
			b = star;
		} else if ((sbody->type == SBody::TYPE_STARPORT_ORBITAL) ||
		           (sbody->type == SBody::TYPE_STARPORT_SURFACE)) {
			SpaceStation *ss = new SpaceStation(sbody);
			b = ss;
		} else {
			Planet *planet = new Planet(sbody);
			b = planet;
		}
		b->SetLabel(sbody->name.c_str());
		b->SetPosition(vector3d(0,0,0));
		AddBody(b);
	}
	f = MakeFrameFor(sbody, b, f);

	for (std::vector<SBody*>::iterator i = sbody->children.begin(); i != sbody->children.end(); ++i) {
		GenBody(*i, f);
	}
}

void BuildSystem()
{
	GenBody(Pi::currentSystem->rootBody, rootFrame);
	rootFrame->SetPosition(vector3d(0,0,0));
	rootFrame->SetVelocity(vector3d(0,0,0));
	rootFrame->UpdateOrbitRails();
}

void AddBody(Body *b)
{
	bodies.push_back(b);
}

void RemoveBody(Body *b)
{
	b->SetFrame(0);
	bodies.remove(b);
}

void KillBody(Body* const b)
{
	if (!b->IsDead()) {
		b->MarkDead();
		if (b != Pi::player) corpses.push_back(b);
	}
}

static bool OnCollision(Object *o1, Object *o2, CollisionContact *c, double relativeVel)
{
	Body *pb1 = static_cast<Body*>(o1);
	Body *pb2 = static_cast<Body*>(o2);
	/* Not always a Body (could be CityOnPlanet, which is a nasty exception I should eradicate) */
	if (o1->IsType(Object::BODY)) {
		if (pb1 && !pb1->OnCollision(o2, c->geomFlag, relativeVel)) return false;
	}
	if (o2->IsType(Object::BODY)) {
		if (pb2 && !pb2->OnCollision(o1, c->geomFlag, relativeVel)) return false;
	}
	return true;
}

static void hitCallback(CollisionContact *c)
{
	//printf("OUCH! %x (depth %f)\n", SDL_GetTicks(), c->depth);

	Object *po1 = static_cast<Object*>(c->userData1);
	Object *po2 = static_cast<Object*>(c->userData2);

	const bool po1_isDynBody = po1->IsType(Object::DYNAMICBODY);
	const bool po2_isDynBody = po2->IsType(Object::DYNAMICBODY);
	// collision response
	assert(po1_isDynBody || po2_isDynBody);

	if (po1_isDynBody && po2_isDynBody) {
		DynamicBody *b1 = static_cast<DynamicBody*>(po1);
		DynamicBody *b2 = static_cast<DynamicBody*>(po2);
		const vector3d linVel1 = b1->GetVelocity();
		const vector3d linVel2 = b2->GetVelocity();
		const vector3d angVel1 = b1->GetAngVelocity();
		const vector3d angVel2 = b2->GetAngVelocity();
		
		const double coeff_rest = 0.5;
		// step back
//		mover->UndoTimestep();

		const double invMass1 = 1.0 / b1->GetMass();
		const double invMass2 = 1.0 / b2->GetMass();
		const vector3d hitPos1 = c->pos - b1->GetPosition();
		const vector3d hitPos2 = c->pos - b2->GetPosition();
		const vector3d hitVel1 = linVel1 + angVel1.Cross(hitPos1);
		const vector3d hitVel2 = linVel2 + angVel2.Cross(hitPos2);
		const double relVel = (hitVel1 - hitVel2).Dot(c->normal);
		// moving away so no collision
		if (relVel > 0) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert1 = 1.0 / b1->GetAngularInertia();
		const double invAngInert2 = 1.0 / b2->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term2 = invMass2;
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal)*invAngInert1).Cross(hitPos1));
		const double term4 = c->normal.Dot((hitPos2.Cross(c->normal)*invAngInert2).Cross(hitPos2));

		const double j = numerator / (term1 + term2 + term3 + term4);
		const vector3d force = j * c->normal;
					
		b1->SetVelocity(linVel1 + force*invMass1);
		b1->SetAngVelocity(angVel1 + hitPos1.Cross(force)*invAngInert1);
		b2->SetVelocity(linVel2 - force*invMass2);
		b2->SetAngVelocity(angVel2 - hitPos2.Cross(force)*invAngInert2);
	} else {
		// one body is static
		vector3d hitNormal;
		DynamicBody *mover;

		if (po1_isDynBody) {
			mover = static_cast<DynamicBody*>(po1);
			hitNormal = c->normal;
		} else {
			mover = static_cast<DynamicBody*>(po2);
			hitNormal = -c->normal;
		}

		const double coeff_rest = 0.5;
		const vector3d linVel1 = mover->GetVelocity();
		const vector3d angVel1 = mover->GetAngVelocity();
		
		// step back
//		mover->UndoTimestep();

		const double invMass1 = 1.0 / mover->GetMass();
		const vector3d hitPos1 = c->pos - mover->GetPosition();
		const vector3d hitVel1 = linVel1 + angVel1.Cross(hitPos1);
		const double relVel = hitVel1.Dot(c->normal);
		// moving away so no collision
		if (relVel > 0 && !c->geomFlag) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert = 1.0 / mover->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal)*invAngInert).Cross(hitPos1));

		const double j = numerator / (term1 + term3);
		const vector3d force = j * c->normal;
					
		mover->SetVelocity(linVel1 + force*invMass1);
		mover->SetAngVelocity(angVel1 + hitPos1.Cross(force)*invAngInert);
	}
}

void CollideFrame(Frame *f)
{
	if (f->m_astroBody && (f->m_astroBody->IsType(Object::TERRAINBODY))) {
		// this is pretty retarded
		for (bodiesIter_t i = bodies.begin(); i!=bodies.end(); ++i) {
			if ((*i)->GetFrame() != f) continue;
			if (!(*i)->IsType(Object::DYNAMICBODY)) continue;
			DynamicBody *dynBody = static_cast<DynamicBody*>(*i);

			Aabb aabb;
			dynBody->GetAabb(aabb);
			const matrix4x4d &trans = dynBody->GetGeom()->GetTransform();

			const vector3d aabbCorners[8] = {
				vector3d(aabb.min.x, aabb.min.y, aabb.min.z),
				vector3d(aabb.min.x, aabb.min.y, aabb.max.z),
				vector3d(aabb.min.x, aabb.max.y, aabb.min.z),
				vector3d(aabb.min.x, aabb.max.y, aabb.max.z),
				vector3d(aabb.max.x, aabb.min.y, aabb.min.z),
				vector3d(aabb.max.x, aabb.min.y, aabb.max.z),
				vector3d(aabb.max.x, aabb.max.y, aabb.min.z),
				vector3d(aabb.max.x, aabb.max.y, aabb.max.z)
			};

			CollisionContact c;

			for (int j=0; j<8; j++) {
				const vector3d &s = aabbCorners[j];
				vector3d pos = trans * s;
				double terrain_height = static_cast<Planet*>(f->m_astroBody)->GetTerrainHeight(pos.Normalized());
				double altitude = pos.Length();
				double hitDepth = terrain_height - altitude;
				if (altitude < terrain_height) {
					c.pos = pos;
					c.normal = pos.Normalized();
					c.depth = hitDepth;
					c.userData1 = static_cast<void*>(dynBody);
					c.userData2 = static_cast<void*>(f->m_astroBody);
					hitCallback(&c);
				}
			}
		}
	}
	f->GetCollisionSpace()->Collide(&hitCallback);
	for (std::list<Frame*>::iterator i = f->m_children.begin(); i != f->m_children.end(); ++i) {
		CollideFrame(*i);
	}
}


void TimeStep(float step)
{
	if (hyperspacingTo) {
		Pi::RequestTimeAccel(6);

		hyperspaceAnim += step;
		if (Pi::GetGameTime() > hyperspaceEndTime) {
			DoHyperspaceTo(0);
			Pi::RequestTimeAccel(1);
			hyperspaceAnim = 0;
		}
		// don't take a physics step at this mental time accel
		return;
	}

	CollideFrame(rootFrame);
	// XXX does not need to be done this often

	// update frames of reference
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i)
		(*i)->UpdateFrame();

	// AI acts here, then move all bodies and frames
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i)
		(*i)->StaticUpdate(step);

	rootFrame->UpdateOrbitRails();

	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i)
		(*i)->TimeStepUpdate(step);
	
	Pi::cpan->TimeStepUpdate(step);

	Sfx::TimeStepAll(step, rootFrame);

	Pi::luaOnEnterSystem->Emit();
	Pi::luaOnLeaveSystem->Emit();
	Pi::luaOnFrameChanged->Emit();
	Pi::luaOnShipHit->Emit();
	Pi::luaOnShipCollided->Emit();
	Pi::luaOnShipDestroyed->Emit();
	Pi::luaOnShipDocked->Emit();
	Pi::luaOnShipAlertChanged->Emit();
	Pi::luaOnShipUndocked->Emit();
	Pi::luaOnShipLanded->Emit();
	Pi::luaOnShipTakeOff->Emit();
	Pi::luaOnJettison->Emit();
	Pi::luaOnAICompleted->Emit();
	Pi::luaOnCreateBB->Emit();
	Pi::luaOnUpdateBB->Emit();
	Pi::luaOnShipFlavourChanged->Emit();
	Pi::luaOnShipEquipmentChanged->Emit();

	Pi::luaTimer->Tick();

	PruneCorpses();
}

void PruneCorpses()
{
	for (bodiesIter_t corpse = corpses.begin(); corpse != corpses.end(); ++corpse) {
		for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i)
			(*i)->NotifyDeleted(*corpse);
		bodies.remove(*corpse);
		delete *corpse;
	}
	corpses.clear();
}

/*
 * Called during play to initiate hyperspace sequence.
 */
void StartHyperspaceTo(Ship *ship, const SystemPath *dest)
{
	int fuelUsage;
	double duration;
	if (!ship->CanHyperspaceTo(dest, fuelUsage, duration)) return;
	ship->UseHyperspaceFuel(dest);
		
	Pi::luaOnLeaveSystem->Queue(ship);

	if (Pi::player == ship) {
		if (Pi::player->GetFlightControlState() == Player::CONTROL_AUTOPILOT)
			Pi::player->SetFlightControlState(Player::CONTROL_MANUAL);

		// Departure clouds going to the same system as us are turned
		// into arrival clouds and stored here
		for (bodiesIter_t i = bodies.begin(); i != bodies.end();) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(*i);
			if ((*i)->IsType(Object::HYPERSPACECLOUD) && (!cloud->IsArrival()) &&
					(cloud->GetShip() != 0)) {
				// only comparing system, not precise body target
				const SystemPath cloudDest = cloud->GetShip()->GetHyperspaceDest();
				if (cloudDest.IsSameSystem(*dest)) {
					Pi::player->NotifyDeleted(cloud);
					cloud->GetShip()->SetHyperspaceDest(Pi::currentSystem->GetPath());
					cloud->SetIsArrival(true);
					cloud->SetFrame(0);
					storedArrivalClouds.push_back(cloud);
					i = bodies.erase(i);
				} else {
					++i;
				}
			} else {
				++i;
			}
		}
		printf("%lu clouds brought over\n", storedArrivalClouds.size());

		Space::Clear();

		hyperspacingTo = new SystemPath(*dest);
		hyperspaceAnim = 0.0f;
		hyperspaceDuration = duration;
		hyperspaceEndTime = Pi::GetGameTime() + duration;

		Pi::player->ClearThrusterState();
		Pi::player->SetFlightState(Ship::HYPERSPACE);

		printf("Started hyperspacing...\n");
	} else {
		// XXX note that cloud now takes ownership of the ship object, and
		// so we can drop the reference in Space::bodies. ship will be freed
		// when the hyperspacecloud is freed
		HyperspaceCloud *cloud = new HyperspaceCloud(ship, Pi::GetGameTime() + duration, false);
		cloud->SetFrame(ship->GetFrame());
		cloud->SetPosition(ship->GetPosition());
		ship->SetFrame(0);

		// need to swap ship out of bodies list, replacing it with
		// cloud
		for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
			if (*i == ship) {
				*i = cloud;
				break;
			}
		}

		if (Pi::player->GetCombatTarget() == ship && !Pi::player->GetNavTarget())
			Pi::player->SetNavTarget(cloud, Pi::player->GetSetSpeedTarget() == ship);

		// Hyperspacing ship must drop references to all other bodies,
		// and they must all drop references to it.
		// make other objects drop their references to this dude
		for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
			if (*i != cloud) {
				(*i)->NotifyDeleted(ship);
				ship->NotifyDeleted(*i);
			}
		}

		ship->SetFlightState(Ship::HYPERSPACE);
	}
}

// random point on a sphere, distributed uniformly by area
vector3d GetRandomPosition(float min_dist, float max_dist)
{
	// see http://mathworld.wolfram.com/SpherePointPicking.html
	// or a Google search for further information
	const double dist = Pi::rng.Double(min_dist, max_dist);
	const double z = Pi::rng.Double_closed(-1.0, 1.0);
	const double theta = Pi::rng.Double(2.0*M_PI);
	const double r = sqrt(1.0 - z*z) * dist;
	return vector3d(r*cos(theta), r*sin(theta), z*dist);
}

vector3d GetPositionAfterHyperspace(const SystemPath *source, const SystemPath *dest)
{
	Sector source_sec(source->sectorX,source->sectorY,source->sectorZ);
	Sector dest_sec(dest->sectorX,dest->sectorY,dest->sectorZ);
	Sector::System source_sys = source_sec.m_systems[source->systemIndex];
	Sector::System dest_sys = dest_sec.m_systems[dest->systemIndex];
	const vector3d sourcePos = vector3d(source_sys.p) + vector3d(source->sectorX, source->sectorY, source->sectorZ);
	const vector3d destPos = vector3d(dest_sys.p) + vector3d(dest->sectorX, dest->sectorY, dest->sectorZ);
	return (sourcePos - destPos).Normalized() * 11.0*AU + GetRandomPosition(5.0,20.0)*1000.0; // "hyperspace zone": 11 AU from primary
}

/*
 * Called at end of hyperspace sequence to place the player in a system.
 */
void DoHyperspaceTo(const SystemPath *dest)
{
	bool isRealHyperspaceEvent = false;
	if (dest == 0) {
		dest = hyperspacingTo;
		isRealHyperspaceEvent = true;
	} else {
		// called with dest indicates start from start point or saved
		// game so don't insert stored arrival clouds into system
		// XXX this shit should all be cleared on new game init....
		for (std::list<HyperspaceCloud*>::iterator i = storedArrivalClouds.begin();
				i != storedArrivalClouds.end(); ++i) {
			delete *i;
		}
		storedArrivalClouds.clear();
	}

	const SystemPath psource = Pi::currentSystem ? Pi::currentSystem->GetPath() : SystemPath(0,0,0,0);
	const SystemPath pdest = dest->SystemOnly();
	Pi::currentSystem = StarSystem::GetCached(dest);
	Space::Clear();
	Space::BuildSystem();
	
	Pi::player->SetFrame(Space::rootFrame);
	Pi::player->SetVelocity(vector3d(0,0,-100.0));
	Pi::player->SetPosition(GetPositionAfterHyperspace(&psource, &pdest));
	Pi::player->SetRotMatrix(matrix4x4d::Identity());
	Pi::player->Enable();
	Pi::player->SetFlightState(Ship::FLYING);

	if (isRealHyperspaceEvent) {
		HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::GetGameTime(), true);
		cloud->SetPosition(Pi::player->GetPosition());
		cloud->SetFrame(Space::rootFrame);
		Space::AddBody(cloud);
	}

	// do stuff to clouds we brought over from the last system
	for (std::list<HyperspaceCloud*>::iterator i = storedArrivalClouds.begin(); i != storedArrivalClouds.end(); ++i) {
		HyperspaceCloud *cloud = *i;

		// first we have to figure out where to put it
		cloud->SetFrame(Space::rootFrame);
		cloud->SetVelocity(vector3d(0,0,0));

		cloud->SetPosition(GetPositionAfterHyperspace(&psource, &pdest));

		Space::AddBody(cloud);

		if (cloud->GetDueDate() < Pi::GetGameTime()) {
			// they emerged from hyperspace some time ago
			Ship *ship = cloud->EvictShip();

			ship->SetFrame(Space::rootFrame);
			ship->SetVelocity(vector3d(0,0,-100.0));
			ship->SetRotMatrix(matrix4x4d::Identity());
			ship->Enable();
			ship->SetFlightState(Ship::FLYING);

			SystemPath sdest = ship->GetHyperspaceDest();
			if (sdest.IsSystemPath()) {
				// travelling to the system as a whole, so just dump them on
				// the cloud - we can't do any better in this case
				ship->SetPosition(cloud->GetPosition());
			}

			else {
				// on their way to a body. they're already in-system so we
				// want to simulate some travel to their destination. we
				// naively assume full accel for half the distance, flip and
				// full brake for the rest.
				Body *target_body = FindBodyForPath(&sdest);
				double dist_to_target = cloud->GetPositionRelTo(target_body).Length();
				double half_dist_to_target = dist_to_target / 2.0;
				double accel = -(ship->GetShipType().linThrust[ShipType::THRUSTER_FORWARD] / ship->GetMass());
				double travel_time = Pi::GetGameTime() - cloud->GetDueDate();

				// I can't help but feel some actual math would do better here
				double speed = 0;
				double dist = 0;
				while (travel_time > 0 && dist <= half_dist_to_target) {
					speed += accel;
					dist += speed;
					travel_time--;
				}
				while (travel_time > 0 && dist < dist_to_target) {
					speed -= accel;
					dist += speed;
					travel_time--;
				}

				if (travel_time <= 0) {
					vector3d pos =
						target_body->GetPositionRelTo(Space::rootFrame) +
						cloud->GetPositionRelTo(target_body).Normalized() * (dist_to_target - dist);
					ship->SetPosition(pos);
				}

				else {
					// ship made it with time to spare. just put it somewhere
					// near the body. the script should be issuing a dock or
					// flyto command in onEnterSystem so it should sort it
					// itself out long before the player can get near
					
					SBody *sbody = Pi::currentSystem->GetBodyByPath(&sdest);
					if (sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(GetRandomPosition(1000.0,1000.0)*1000.0); // somewhere 1000km out
					}

					else {
						if (sbody->type == SBody::TYPE_STARPORT_SURFACE) {
							sbody = sbody->parent;
							SystemPath path = Pi::currentSystem->GetPathOf(sbody);
							target_body = FindBodyForPath(&path);
						}

						double sdist = sbody->GetRadius()*2.0;

						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(GetRandomPosition(sdist,sdist));
					}
				}
			}

			Space::AddBody(ship);

			Pi::luaOnEnterSystem->Queue(ship);
		}
	}
	storedArrivalClouds.clear();

	// bit of a hack, this should be only false if DoHyperspaceTo is used at
	// game startup (eg debug point)
	if (Pi::IsGameStarted())
		Pi::luaOnEnterSystem->Queue(Pi::player);
	
	delete hyperspacingTo;
	hyperspacingTo = 0;
	
	Pi::sectorView->ResetHyperspaceTarget();
}

/* called at game start to load the system and put the player in a starport */
void SetupSystemForGameStart(const SystemPath *dest, int starport, int port)
{
	Pi::currentSystem = StarSystem::GetCached(dest);
	Space::Clear();
	Space::BuildSystem();

	SpaceStation *station = 0;
	for (Space::bodiesIter_t i = Space::bodies.begin(); i!=Space::bodies.end(); i++) {
		if ((*i)->IsType(Object::SPACESTATION) && !starport--) {
			station = static_cast<SpaceStation*>(*i);
			break;
		}
	}
	assert(station);

	Pi::player->Enable();
	Pi::player->SetPosition(vector3d(0,0,0)); 
	Pi::player->SetVelocity(vector3d(0,0,0));

	Pi::player->SetFrame(station->GetFrame()); 
	Pi::player->SetDockedWith(station, port); 

	station->CreateBB();
}

float GetHyperspaceAnim()
{
	return hyperspaceAnim;
}

const SystemPath *GetHyperspaceDest()
{
	return hyperspacingTo;
}

double GetHyperspaceDuration()
{
	return hyperspaceDuration;
}

}
