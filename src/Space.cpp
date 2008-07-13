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
#include "sbre/sbre.h"

dWorldID Space::world;
std::list<Body*> Space::bodies;
Frame *Space::rootFrame;
static dJointGroupID _contactgroup;
std::list<Body*> Space::corpses;

void Space::Init()
{
	world = dWorldCreate();
	rootFrame = new Frame(NULL, "System");
	rootFrame->SetRadius(FLT_MAX);
	_contactgroup = dJointGroupCreate(0);
	//dWorldSetGravity(world, 0,-9.81,0);
}

void Space::Clear()
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->SetFrame(NULL);
		if ((*i) != (Body*)Pi::player) {
			KillBody(*i);
		}
	}
	PruneCorpses();

	for (std::list<Frame*>::iterator i = rootFrame->m_children.begin(); i != rootFrame->m_children.end(); ++i) delete *i;
	rootFrame->m_children.clear();

	Pi::player->SetFrame(rootFrame);
}

void Space::GenBody(StarSystem *system, StarSystem::SBody *sbody, Frame *f)
{
	Body *b;
	if (sbody->type == StarSystem::SBody::TYPE_STAR) {
		Star *star = new Star(sbody->subtype);
		star->SetRadius(sbody->radius);
		b = star;
	} else {
		Planet *planet = new Planet(sbody->subtype);
		planet->SetRadius(sbody->radius);
		b = planet;
	}
	b->SetLabel(sbody->name.c_str());
	
	Frame *myframe;
	if (sbody->parent) {
		myframe = new Frame(f, sbody->name.c_str());
		vector3d pos = sbody->orbit.CartesianPosAtTime(0);
		myframe->SetPosition(pos);
		myframe->SetRadius(10*sbody->radius);
		b->SetFrame(myframe);
	} else {
		b->SetFrame(f);
		myframe = f;
	}

	b->SetPosition(vector3d(0,0,0));
	
	AddBody(b);

	for (std::vector<StarSystem::SBody*>::iterator i = sbody->children.begin(); i != sbody->children.end(); ++i) {
		GenBody(system, *i, myframe);
	}
}

void Space::BuildSystem(StarSystem *system)
{
	GenBody(system, system->rootBody, rootFrame);
}

void Space::AddBody(Body *b)
{
	bodies.push_back(b);
}

void Space::KillBody(Body* const b)
{
	b->MarkDead();
	corpses.push_back(b);
}

void Space::UpdateFramesOfReference()
{
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		Body *b = *i;

		if (!b->GetFlags() & Body::FLAG_CAN_MOVE_FRAME) continue;

		// falling out of frames
		if (!b->GetFrame()->IsLocalPosInFrame(b->GetPosition())) {
			printf("%s leaves frame %s\n", b->GetLabel().c_str(), b->GetFrame()->GetLabel());
			
			Frame *new_frame = b->GetFrame()->m_parent;
			if (new_frame) { // don't let fall out of root frame
				vector3d new_pos = b->GetPositionRelTo(new_frame);
				b->SetFrame(new_frame);
				b->SetPosition(new_pos);
			}
		}

		// entering into frames
		for (std::list<Frame*>::iterator j = b->GetFrame()->m_children.begin(); j != b->GetFrame()->m_children.end(); ++j) {
			Frame *kid = *j;
			vector3d pos = b->GetFrame()->GetPosRelativeToOtherFrame(kid) + b->GetPosition();
			if (kid->IsLocalPosInFrame(pos)) {
				printf("%s enters frame %s\n", b->GetLabel().c_str(), kid->GetLabel());
				b->SetPosition(pos);
				b->SetFrame(kid);
				break;
			}
		}
	}
}

/*
 * return false if ode is not to apply collision
 */
static bool _OnCollision(dGeomID g1, dGeomID g2, Object *o1, Object *o2, int numContacts, dContact contacts[])
{
	if ((o1->GetType() == Object::LASER) || (o2->GetType() == Object::LASER)) {
		if (o1->GetType() == Object::LASER) {
			std::swap<Object*>(o1, o2);
			std::swap<dGeomID>(g1, g2);
		}
		Ship::LaserObj *lobj = static_cast<Ship::LaserObj*>(o2);
		if (o1 == lobj->owner) return false;
		printf("%s (geom flag %x) was shot by %s\n", ((ModelBody::Geom*)o1)->parent->GetLabel().c_str(), 
			((ModelBody::Geom*)o1)->flags, lobj->owner->GetLabel().c_str());

		if (o1->GetType() == Object::SHIP) {
			DynamicBody *rb = (DynamicBody*)o1;
			dVector3 start,dir;
			dGeomRayGet(g2, start, dir);
			dBodyAddForceAtPos(rb->m_body,
				100*dir[0],
				100*dir[1],
				100*dir[2],
				contacts[0].geom.pos[0],
				contacts[0].geom.pos[1],
				contacts[0].geom.pos[2]);
		}

		return false;
	} else {
		Body *pb1, *pb2;
		int flags = 0;
		// geom bodies point to their parents
		if (o1->GetType() == Object::GEOM) {
			pb1 = static_cast<ModelBody::Geom*>(o1)->parent;
			flags |= static_cast<ModelBody::Geom*>(o1)->flags;
		} else pb1 = static_cast<Body*>(o1);
		if (o2->GetType() == Object::GEOM) {
			pb2 = static_cast<ModelBody::Geom*>(o2)->parent;
			flags |= static_cast<ModelBody::Geom*>(o2)->flags;
		} else pb2 = static_cast<Body*>(o2);

		printf("Collision flags %x\n", flags);
		if ((pb1 && !pb1->OnCollision(pb2)) || (pb2 && !pb2->OnCollision(pb1))) return false;
	}
	return true;
}

static void nearCallback(void *data, dGeomID o0, dGeomID o1)
{
	// Create an array of dContact objects to hold the contact joints
	static const int MAX_CONTACTS = 100;
	dContact contact[MAX_CONTACTS];

	for (int i = 0; i < MAX_CONTACTS; i++)
	{
		contact[i].surface.mode = dContactBounce;
		contact[i].surface.mu = 0;
		contact[i].surface.mu2 = 0;
		contact[i].surface.bounce = 0.1;
		contact[i].surface.bounce_vel = 0.1;
	}
	if (int numc = dCollide(o0, o1, MAX_CONTACTS, &contact[0].geom, sizeof(dContact)))
	{
		printf("%d contacts\n", numc);
		// don't ye get confused between Pi Body class and libODE bodies
		Object *po1 = static_cast<Object*>(dGeomGetData(o0));
		Object *po2 = static_cast<Object*>(dGeomGetData(o1));
		if (!_OnCollision(o0, o1, po1, po2, numc, contact)) return;
		// Get the dynamics body for each geom
		dBodyID b1 = dGeomGetBody(o0);
		dBodyID b2 = dGeomGetBody(o1);
		// To add each contact point found to our joint group we call dJointCreateContact which is just one of the many
		// different joint types available.  
		for (int i = 0; i < numc; i++)
		{
			// dJointCreateContact needs to know which world and joint group to work with as well as the dContact
			// object itself. It returns a new dJointID which we then use with dJointAttach to finally create the
			// temporary contact joint between the two geom bodies.
			dJointID c = dJointCreateContact(Space::world, _contactgroup, contact + i);
/*			struct dContactGeom {
  dVector3 pos;       // contact position
  dVector3 normal;    // normal vector
  dReal depth;        // penetration depth
  dGeomID g1,g2;      // the colliding geoms
};*/
			dJointAttach(c, b1, b2);
		}
	}	
}

void Space::CollideFrame(Frame *f)
{
	dSpaceCollide(f->GetSpaceID(), NULL, &nearCallback);
	for (std::list<Frame*>::iterator i = f->m_children.begin(); i != f->m_children.end(); ++i) {
		CollideFrame(*i);
	}
}

void Space::TimeStep(float step)
{
	CollideFrame(rootFrame);
	dWorldQuickStep(world, step);
	dJointGroupEmpty(_contactgroup);
	// XXX does not need to be done this often
	UpdateFramesOfReference();
	
	for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i) {
		(*i)->TimeStepUpdate(step);
	}

	PruneCorpses();
}

void Space::PruneCorpses()
{
	for (bodiesIter_t corpse = corpses.begin(); corpse != corpses.end(); ++corpse) {
		for (bodiesIter_t i = bodies.begin(); i != bodies.end(); ++i)
			(*i)->NotifyDeath(*corpse);
		bodies.remove(*corpse);
		delete *corpse;
	}
	corpses.clear();
}

struct body_zsort_t {
	double dist;
	Body *b;
};

struct body_zsort_compare : public std::binary_function<body_zsort_t, body_zsort_t, bool> {
	bool operator()(body_zsort_t a, body_zsort_t b) { return a.dist > b.dist; }
};

void Space::Render(const Frame *cam_frame)
{
	// simple z-sort!!!!!!!!!!!!!11
	body_zsort_t *bz = new body_zsort_t[bodies.size()];
	int idx = 0;
	for (std::list<Body*>::iterator i = bodies.begin(); i != bodies.end(); ++i) {
		vector3d toBody = (*i)->GetPositionRelTo(cam_frame);
		bz[idx].dist = toBody.Length();
		bz[idx].b = *i;
		idx++;
	}
	sort(bz, bz+bodies.size(), body_zsort_compare());

	// Probably the right place for this when partitioning is done
	sbreSetDepthRange (Pi::GetScrWidth()*0.5f, 0.0f, 1.0f);

	for (unsigned int i=0; i<bodies.size(); i++) {
		bz[i].b->Render(cam_frame);
	}
	delete [] bz;
}
