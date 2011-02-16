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
#include "PiLuaModules.h"
#include "Render.h"

namespace Space {

std::list<Body*> bodies;
Frame *rootFrame;
static void UpdateFramesOfReference();
static void CollideFrame(Frame *f);
static void PruneCorpses();
static void ApplyGravity();
static std::list<Body*> corpses;
static SBodyPath *hyperspacingTo;
static float hyperspaceAnim;
static double hyperspaceEndTime;
static std::list<HyperspaceCloud*> storedArrivalClouds;
static bool beingBuilt;

void Init()
{
	rootFrame = new Frame(NULL, "System");
	rootFrame->SetRadius(FLT_MAX);
}

void Clear()
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->SetFrame(NULL);
		if ((*i) != (Body*)Pi::player) {
			KillBody(*i);
		}
	}
	PruneCorpses();

	Pi::player->SetFrame(rootFrame);
	for (std::list<Frame*>::iterator i = rootFrame->m_children.begin(); i != rootFrame->m_children.end(); ++i) delete *i;
	rootFrame->m_children.clear();
	rootFrame->m_astroBody = 0;
	rootFrame->m_sbody = 0;
}

bool IsSystemBeingBuilt()
{
	return beingBuilt;
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

Body *FindBodyForSBodyPath(const SBodyPath *path)
{
	// it is a bit dumb that currentSystem is not part of Space...
	SBody *body = Pi::currentSystem->GetBodyByPath(path);

	if (!body) return 0;

	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->GetSBody() == body) return *i;
	}
	return 0;
}

void RadiusDamage(Body *attacker, Frame *f, const vector3d &pos, double radius, double kgDamage)
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		if ((*i)->GetFrame() != f) continue;
		double dist = ((*i)->GetPosition() - pos).Length();
		if (dist < radius) {
			// linear damage decay with distance
			(*i)->OnDamage(attacker, kgDamage * (radius - dist) / radius);
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
		wr.Double(hyperspaceEndTime);
	}
}

void Unserialize(Serializer::Reader &rd)
{
	Serializer::IndexSystemBodies(Pi::currentSystem);
	
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
		hyperspacingTo = new SBodyPath;
		SBodyPath::Unserialize(rd, hyperspacingTo);
		hyperspaceAnim = rd.Float();
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
		orbFrame->SetRadius(frameRadius ? frameRadius : 10*sbody->GetRadius());
		//printf("\t\t\t%s has frame size %.0fkm, body radius %.0fkm\n", sbody->name.c_str(),
		//	(frameRadius ? frameRadius : 10*sbody->GetRadius())*0.001f,
		//	sbody->GetRadius()*0.001f);
	
		assert(sbody->GetRotationPeriod() != 0);
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
		orbFrame->SetRadius(frameRadius ? frameRadius : 10*sbody->GetRadius());
	
		assert(sbody->GetRotationPeriod() != 0);
		rotFrame = new Frame(orbFrame, sbody->name.c_str());
		rotFrame->SetRadius(5000.0);//(1.1*sbody->GetRadius());
		rotFrame->SetAngVelocity(vector3d(0.0,(double)static_cast<SpaceStation*>(b)->GetDesiredAngVel(),0.0));
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
		if (planet->GetTerrainHeight(pos) - planet->GetSBody()->GetRadius() == 0.0) {
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

void UpdateFramesOfReference()
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		Body *b = *i;

		if (!(b->GetFlags() & Body::FLAG_CAN_MOVE_FRAME)) continue;

		// falling out of frames
		if (!b->GetFrame()->IsLocalPosInFrame(b->GetPosition())) {
			printf("%s leaves frame %s\n", b->GetLabel().c_str(), b->GetFrame()->GetLabel());
			
			vector3d oldFrameVel = b->GetFrame()->GetVelocity();
			
			Frame *new_frame = b->GetFrame()->m_parent;
			if (new_frame) { // don't let fall out of root frame
				matrix4x4d m = matrix4x4d::Identity();
				b->GetFrame()->ApplyLeavingTransform(m);

				vector3d new_pos = m * b->GetPosition();//b->GetPositionRelTo(new_frame);

				matrix4x4d rot;
				b->GetRotMatrix(rot);
				b->SetRotMatrix(m * rot);
				
				b->SetVelocity(oldFrameVel + m.ApplyRotationOnly(b->GetVelocity() - 
					b->GetFrame()->GetStasisVelocityAtPosition(b->GetPosition())));

				b->SetFrame(new_frame);
				b->SetPosition(new_pos);
			} else {
				b->SetVelocity(b->GetVelocity() + oldFrameVel);
			}
		}

		// entering into frames
		for (std::list<Frame*>::iterator j = b->GetFrame()->m_children.begin(); j != b->GetFrame()->m_children.end(); ++j) {
			Frame *kid = *j;
			matrix4x4d m;
			Frame::GetFrameTransform(b->GetFrame(), kid, m);
			vector3d pos = m * b->GetPosition();
			if (kid->IsLocalPosInFrame(pos)) {
				printf("%s enters frame %s\n", b->GetLabel().c_str(), kid->GetLabel());
				b->SetPosition(pos);
				b->SetFrame(kid);

				matrix4x4d rot;
				b->GetRotMatrix(rot);
				b->SetRotMatrix(m * rot);
				
				// get rid of transforms
				m.ClearToRotOnly();
				b->SetVelocity(m*b->GetVelocity()
					- kid->GetVelocity()
					+ kid->GetStasisVelocityAtPosition(pos));
				break;
			}
		}
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
		const vector3d hitVel1 = linVel1 + vector3d::Cross(angVel1, hitPos1);
		const vector3d hitVel2 = linVel2 + vector3d::Cross(angVel2, hitPos2);
		const double relVel = vector3d::Dot(hitVel1 - hitVel2, c->normal);
		// moving away so no collision
		if (relVel > 0) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert1 = 1.0 / b1->GetAngularInertia();
		const double invAngInert2 = 1.0 / b2->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term2 = invMass2;
		const double term3 = vector3d::Dot(c->normal, vector3d::Cross(vector3d::Cross(hitPos1, c->normal)*invAngInert1, hitPos1));
		const double term4 = vector3d::Dot(c->normal, vector3d::Cross(vector3d::Cross(hitPos2, c->normal)*invAngInert2, hitPos2));

		const double j = numerator / (term1 + term2 + term3 + term4);
		const vector3d force = j * c->normal;
					
		b1->SetVelocity(linVel1 + force*invMass1);
		b1->SetAngVelocity(angVel1 + vector3d::Cross(hitPos1, force)*invAngInert1);
		b2->SetVelocity(linVel2 - force*invMass2);
		b2->SetAngVelocity(angVel2 - vector3d::Cross(hitPos2, force)*invAngInert2);
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
		const vector3d hitVel1 = linVel1 + vector3d::Cross(angVel1, hitPos1);
		const double relVel = vector3d::Dot(hitVel1, c->normal);
		// moving away so no collision
		if (relVel > 0) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert = 1.0 / mover->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term3 = vector3d::Dot(c->normal, vector3d::Cross(vector3d::Cross(hitPos1, c->normal)*invAngInert, hitPos1));

		const double j = numerator / (term1 + term3);
		const vector3d force = j * c->normal;
					
		mover->SetVelocity(linVel1 + force*invMass1);
		mover->SetAngVelocity(angVel1 + vector3d::Cross(hitPos1, force)*invAngInert);
	}
}

void CollideFrame(Frame *f)
{
	if (f->m_astroBody && (f->m_astroBody->IsType(Object::PLANET))) {
		// this is pretty retarded
		for (bodiesIter_t i = bodies.begin(); i!=bodies.end(); ++i) {
			if ((*i)->GetFrame() != f) continue;
			if (!(*i)->IsType(Object::DYNAMICBODY)) continue;
			DynamicBody *dynBody = (DynamicBody*)(*i);

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

			for (int i=0; i<8; i++) {
				const vector3d &s = aabbCorners[i];
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

void ApplyGravity()
{
	Body *lump = 0;
	// gravity is applied when our frame contains an 'astroBody', ie a star or planet,
	// or when our frame contains a rotating frame which contains this body.
	if (Pi::player->GetFrame()->m_astroBody) {
		lump = Pi::player->GetFrame()->m_astroBody;
	} else if (Pi::player->GetFrame()->m_sbody &&
		(Pi::player->GetFrame()->m_children.begin() !=
	           Pi::player->GetFrame()->m_children.end())) {

		lump = (*Pi::player->GetFrame()->m_children.begin())->m_astroBody;
	}
	// just to crap in the player's frame
	if (lump) { 
		for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
			if ((*i)->GetFrame() != Pi::player->GetFrame()) continue;
			if (!(*i)->IsType(Object::DYNAMICBODY)) continue;

			vector3d b1b2 = lump->GetPosition() - (*i)->GetPosition();
			const double m1m2 = (*i)->GetMass() * lump->GetMass();
			const double r = b1b2.Length();
			const double force = G*m1m2 / (r*r);
			b1b2 = b1b2.Normalized() * force;
			static_cast<DynamicBody*>(*i)->AddForce(b1b2);
		}
	}

}

void TimeStep(float step)
{
	if (hyperspacingTo) {
		Pi::RequestTimeAccel(6);

		hyperspaceAnim += step;
		if (Pi::GetGameTime() > hyperspaceEndTime) {
			beingBuilt = true;
			DoHyperspaceTo(0);
			Pi::RequestTimeAccel(1);
			hyperspaceAnim = 0;
			/* Event must be run right now (so 'beingBuilt' is correct) */
			PiLuaModules::QueueEvent("onEnterSystem");
			PiLuaModules::EmitEvents();
			beingBuilt = false;
		}
		// don't take a physics step at this mental time accel
		return;
	}

	ApplyGravity();
	CollideFrame(rootFrame);
	// XXX does not need to be done this often
	UpdateFramesOfReference();
	rootFrame->UpdateOrbitRails();
	
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->TimeStepUpdate(step);
	}
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->StaticUpdate(step);
	}
	Sfx::TimeStepAll(step, rootFrame);

	PiLuaModules::EmitEvents();

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

static bool jumped_within_same_system;

/*
 * Called during play to initiate hyperspace sequence.
 */
void StartHyperspaceTo(Ship *ship, const SBodyPath *dest)
{
	int fuelUsage;
	double duration;
	if (!ship->CanHyperspaceTo(dest, fuelUsage, duration)) return;
	ship->UseHyperspaceFuel(dest);
		
	if (Pi::player == ship) {
		// Departure clouds going to the same system as us are turned
		// into arrival clouds and stored here
		for (bodiesIter_t i = bodies.begin(); i != bodies.end();) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(*i);
			if ((*i)->IsType(Object::HYPERSPACECLOUD) && (!cloud->IsArrival()) &&
					(cloud->GetShip() != 0)) {
				// only comparing system, not precise body target
				SysLoc cloudDest = *((SysLoc*)cloud->GetShip()->GetHyperspaceTarget());
				if (cloudDest == *(SysLoc*)dest) {
					Pi::player->NotifyDeleted(cloud);
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
		printf("%d clouds brought over\n", storedArrivalClouds.size());

		Space::Clear();
		if (!hyperspacingTo) hyperspacingTo = new SBodyPath;
		*hyperspacingTo = *dest;
		hyperspaceAnim = 0.0f;
		hyperspaceEndTime = Pi::GetGameTime() + duration;
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
		// Hyperspacing ship must drop references to all other bodies,
		// and they must all drop references to it.
		// make other objects drop their references to this dude
		for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
			if (*i != cloud) {
				(*i)->NotifyDeleted(ship);
				ship->NotifyDeleted(*i);
			}
		}
	}
}

/* What else can i name it? */
static void PostHyperspacePositionBody(Body *b, Frame *f)
{
	float longitude = Pi::rng.Double(M_PI);
	float latitude = Pi::rng.Double(M_PI);
	float dist = (5.0 + Pi::rng.Double(1.0)) * AU;
	b->SetPosition(vector3d(sin(longitude)*cos(latitude)*dist,
			sin(latitude)*dist,
			cos(longitude)*cos(latitude)*dist));
	b->SetRotMatrix(matrix4x4d::Identity());
	b->SetVelocity(vector3d(0.0,0.0,0.0));
	b->SetFrame(f);
}

/*
 * Called at end of hyperspace sequence or at start of game
 * to place the player in a system.
 */
void DoHyperspaceTo(const SBodyPath *dest)
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
	
	if (Pi::currentSystem) delete Pi::currentSystem;
	Pi::currentSystem = new StarSystem(dest->sectorX, dest->sectorY, dest->systemNum);
	Space::Clear();
	Space::BuildSystem();
	SBody *targetBody = Pi::currentSystem->GetBodyByPath(dest);
	Frame *pframe;
       	if (targetBody->type == SBody::TYPE_STARPORT_SURFACE) {
		pframe = Space::GetFrameWithSBody(targetBody->parent);
	} else {
		pframe = Space::GetFrameWithSBody(targetBody);
	}
	assert(pframe);
	
	PostHyperspacePositionBody(Pi::player, pframe);
	Pi::player->SetVelocity(vector3d(0.0,0.0,-1000.0));
	Pi::player->Enable();
	Pi::player->SetFlightState(Ship::FLYING);

	if (isRealHyperspaceEvent) {
		HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::GetGameTime(), true);
		cloud->SetPosition(Pi::player->GetPosition());
		cloud->SetFrame(pframe);
		Space::AddBody(cloud);
	}

	/* XXX XXX need to put these in an appropriate place in the system and
	 * have some way of the player navigating to them */
	double xoffset = 2000.0;
	for (std::list<HyperspaceCloud*>::iterator i = storedArrivalClouds.begin();
			i != storedArrivalClouds.end(); ++i) {
		if ((*i)->GetDueDate() < Pi::GetGameTime()) {
			// too late dude
			delete *i;
		} else {
			// If the player has closen to follow this hypercloud
			// then put it near the player's destination,
			// otherwise at random loc
			if (Pi::player->GetHyperspaceCloudTargetId() == (*i)->GetId()) {
				PostHyperspacePositionBody(*i, Pi::player->GetFrame());
				(*i)->SetPosition(Pi::player->GetPosition() + vector3d(xoffset,0,0));
				xoffset += 2000.0;
			} else {
				SBody *b = Pi::currentSystem->GetBodyByPath((*i)->GetShip()->GetHyperspaceTarget());
				Frame *f = (b->type == SBody::TYPE_STARPORT_SURFACE ?
						Space::GetFrameWithSBody(b->parent) :
						Space::GetFrameWithSBody(b));
				PostHyperspacePositionBody(*i, f);
			}
			Space::AddBody(*i);
		}
	}
	storedArrivalClouds.clear();

	Pi::onPlayerHyperspaceToNewSystem.emit();
	
	delete hyperspacingTo;
	hyperspacingTo = 0;
	
}

float GetHyperspaceAnim()
{
	return hyperspaceAnim;
}

struct body_zsort_t {
	double dist;
	vector3d viewCoords;
	matrix4x4d viewTransform;
	Body *b;
	Uint32 bodyFlags;
};

struct body_zsort_compare : public std::binary_function<body_zsort_t, body_zsort_t, bool> {
	bool operator()(body_zsort_t a, body_zsort_t b)
	{
		if (a.bodyFlags & Body::FLAG_DRAW_LAST) {
			if (!(b.bodyFlags & Body::FLAG_DRAW_LAST)) return false;
		} else {
			if (b.bodyFlags & Body::FLAG_DRAW_LAST) return true;
		}
		return a.dist > b.dist;
	}
};

void Render(const Frame *cam_frame)
{
	Plane planes[6];
	GetFrustum(planes);

	// simple z-sort!!!!!!!!!!!!!11
	body_zsort_t *bz = new body_zsort_t[bodies.size()];
	int idx = 0;
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		const vector3d pos = (*i)->GetInterpolatedPosition();
		Frame::GetFrameRenderTransform((*i)->GetFrame(), cam_frame, bz[idx].viewTransform);
		vector3d toBody = bz[idx].viewTransform * pos;
		bz[idx].viewTransform = bz[idx].viewTransform;
		bz[idx].viewCoords = toBody;
		bz[idx].dist = toBody.Length();
		bz[idx].bodyFlags = (*i)->GetFlags();
		bz[idx].b = *i;
		idx++;
	}
	sort(bz, bz+bodies.size(), body_zsort_compare());

	for (unsigned int i=0; i<bodies.size(); i++) {
		double boundingRadius = bz[i].b->GetBoundingRadius();

		// test against all frustum planes except far plane
		bool do_draw = true;
		// always render stars (they have a huge glow). Other things do frustum cull
		if (!bz[i].b->IsType(Object::STAR)) {
			for (int p=0; p<5; p++) {
				if (planes[p].DistanceToPoint(bz[i].viewCoords)+boundingRadius < 0) {
					do_draw = false;
					break;
				}
			}
		}
		if (do_draw) bz[i].b->Render(bz[i].viewCoords, bz[i].viewTransform);
	}
	Sfx::RenderAll(rootFrame, cam_frame);
	Render::State::UseProgram(0);
	Render::UnbindAllBuffers();

	delete [] bz;
}

}

