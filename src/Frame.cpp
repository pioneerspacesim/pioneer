// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Frame.h"

#include "GameSaveError.h"
#include "Json.h"
#include "JsonUtils.h"
#include "Sfx.h"
#include "Space.h"
#include "collider/CollisionSpace.h"
#include "utils.h"

std::vector<Frame> Frame::s_frames;
std::vector<CollisionSpace *> Frame::s_collisionSpaces;

Frame::Frame(const Dummy &d, FrameId parent, const char *label, unsigned int flags, double radius) :
	m_parent(parent),
	m_sbody(nullptr),
	m_astroBody(nullptr),
	m_pos(vector3d(0.0)),
	m_initialOrient(matrix3x3d::Identity()),
	m_orient(matrix3x3d::Identity()),
	m_vel(vector3d(0.0)),
	m_angSpeed(0.0),
	m_radius(radius),
	m_flags(flags)
{
	if (!d.madeWithFactory)
		Error("Frame ctor called directly!\n");

	m_thisId = s_frames.size();

	ClearMovement();

	m_collisionSpace.reset(new CollisionSpace());
	s_collisionSpaces.emplace_back(m_collisionSpace.get());
	// TODO: this hacks around TraceRay being called on an uninitialized collision space and crashing
	// Need to further evaluate the impact of this and whether it should be called in the CollisionSpace constructor instead
	// FIXME: this causes a crash when resizing the collisionSpace array because a collision space double-frees when move constructing
	// s_collisionSpaces.back().RebuildObjectTrees();

	if (m_parent.valid())
		Frame::GetFrame(m_parent)->AddChild(m_thisId);
	if (label)
		m_label = label;
}

Frame::Frame(const Dummy &d, FrameId parent) :
	m_parent(parent),
	m_sbody(nullptr),
	m_astroBody(nullptr),
	m_pos(vector3d(0.0)),
	m_initialOrient(matrix3x3d::Identity()),
	m_orient(matrix3x3d::Identity()),
	m_vel(vector3d(0.0)),
	m_angSpeed(0.0),
	m_label("camera"),
	m_radius(0.0),
	m_flags(FLAG_ROTATING),
	m_collisionSpace(nullptr)
{
	if (!d.madeWithFactory)
		Error("Frame ctor called directly!\n");

	m_thisId = s_frames.size();

	ClearMovement();
	if (m_parent.valid())
		Frame::GetFrame(m_parent)->AddChild(m_thisId);
}

Frame::Frame(Frame &&other) noexcept :
	m_sfx(std::move(other.m_sfx)),
	m_thisId(other.m_thisId),
	m_parent(other.m_parent),
	m_children(std::move(other.m_children)),
	m_sbody(other.m_sbody),
	m_astroBody(other.m_astroBody),
	m_pos(other.m_pos),
	m_oldPos(other.m_oldPos),
	m_interpPos(other.m_interpPos),
	m_initialOrient(other.m_initialOrient),
	m_orient(other.m_orient),
	m_vel(other.m_vel),
	m_angSpeed(other.m_angSpeed),
	m_oldAngDisplacement(other.m_oldAngDisplacement),
	m_label(std::move(other.m_label)),
	m_radius(other.m_radius),
	m_flags(other.m_flags),
	m_collisionSpace(other.m_collisionSpace.release()),
	m_rootVel(other.m_rootVel),
	m_rootPos(other.m_rootPos),
	m_rootOrient(other.m_rootOrient),
	m_rootInterpPos(other.m_rootInterpPos),
	m_rootInterpOrient(other.m_rootInterpOrient),
	m_astroBodyIndex(other.m_astroBodyIndex),
	d(other.d)
{
	other.d.madeWithFactory = true;
}

Frame &Frame::operator=(Frame &&other)
{
	m_sfx = std::move(other.m_sfx);
	m_thisId = other.m_thisId;
	m_parent = other.m_parent;
	m_children = std::move(other.m_children);
	m_sbody = other.m_sbody;
	m_astroBody = other.m_astroBody;
	m_pos = other.m_pos;
	m_oldPos = other.m_oldPos;
	m_interpPos = other.m_interpPos;
	m_initialOrient = other.m_initialOrient;
	m_orient = other.m_orient;
	m_vel = other.m_vel;
	m_angSpeed = other.m_angSpeed;
	m_oldAngDisplacement = other.m_oldAngDisplacement;
	m_label = std::move(other.m_label);
	m_radius = other.m_radius;
	m_flags = other.m_flags;
	m_collisionSpace.reset(other.m_collisionSpace.release());
	m_rootVel = other.m_rootVel;
	m_rootPos = other.m_rootPos;
	m_rootOrient = other.m_rootOrient;
	m_rootInterpPos = other.m_rootInterpPos;
	m_rootInterpOrient = other.m_rootInterpOrient;
	m_astroBodyIndex = other.m_astroBodyIndex;
	d = other.d;
	return *this;
}

void Frame::ToJson(Json &frameObj, FrameId fId, Space *space)
{
	Frame *f = Frame::GetFrame(fId);

	assert(f != nullptr);

	frameObj["frameId"] = f->m_thisId;
	frameObj["flags"] = f->m_flags;
	frameObj["radius"] = f->m_radius;
	frameObj["label"] = f->m_label;
	frameObj["pos"] = f->m_pos;
	frameObj["ang_speed"] = f->m_angSpeed;
	frameObj["init_orient"] = f->m_initialOrient;
	frameObj["index_for_system_body"] = space->GetIndexForSystemBody(f->m_sbody);
	frameObj["index_for_astro_body"] = space->GetIndexForBody(f->m_astroBody);

	Json childFrameArray = Json::array(); // Create JSON array to contain child frame data.
	for (FrameId kid : f->GetChildren()) {
		Json childFrameArrayEl = Json::object(); // Create JSON object to contain child frame.
		Frame::ToJson(childFrameArrayEl, kid, space);
		childFrameArray.push_back(childFrameArrayEl); // Append child frame object to array.
	}
	if (!childFrameArray.empty())
		frameObj["child_frames"] = childFrameArray; // Add child frame array to frame object.

	// Add sfx array to supplied object.
	SfxManager::ToJson(frameObj, f->m_thisId);
}

Frame::~Frame()
{
	if (!d.madeWithFactory) {
		Error("Frame instance deletion outside 'DeleteFrame' [%zu]\n", m_thisId.id());
	}
}

FrameId Frame::CreateFrame(FrameId parent, const char *label, unsigned int flags, double radius)
{
	Dummy dummy;
	dummy.madeWithFactory = true;

	s_frames.emplace_back(dummy, parent, label, flags, radius);
	return (s_frames.size() - 1);
}

FrameId Frame::FromJson(const Json &frameObj, Space *space, FrameId parent, double at_time)
{
	Dummy dummy;
	dummy.madeWithFactory = true;

	// Set parent to nullptr here in order to avoid this frame
	// being a child twice (due to ctor calling AddChild)
	s_frames.emplace_back(dummy, FrameId(), nullptr);

	Frame *f = &s_frames.back();

	f->m_parent = parent;
	f->d.madeWithFactory = false;

	try {
		f->m_thisId = frameObj["frameId"];

		// Check if frames order in load and save are the same
		assert(s_frames.size() == 0 || (s_frames.size() - 1) == f->m_thisId.id());

		f->m_flags = frameObj["flags"];
		f->m_radius = frameObj["radius"];
		f->m_label = frameObj["label"].get<std::string>();

		f->m_pos = frameObj["pos"];
		f->m_angSpeed = frameObj["ang_speed"];
		f->SetInitialOrient(frameObj["init_orient"], at_time);
		f->m_sbody = space->GetSystemBodyByIndex(frameObj["index_for_system_body"]);
		f->m_astroBodyIndex = frameObj["index_for_astro_body"];
		f->m_vel = vector3d(0.0); // m_vel is set to zero.

		if (frameObj.count("child_frames") && frameObj["child_frames"].is_array()) {
			Json childFrameArray = frameObj["child_frames"];
			f->m_children.reserve(childFrameArray.size());
			for (unsigned int i = 0; i < childFrameArray.size(); ++i) {
				// During 'FromJson' a reallocation may happens, invalidating 'f',
				// thus store his FrameId and renew it
				FrameId temp = f->m_thisId;
				FrameId kidId = FromJson(childFrameArray[i], space, f->m_thisId, at_time);
				f = &s_frames[temp];
				f->m_children.push_back(kidId);
			}
		} else {
			f->m_children.clear();
		}
	} catch (Json::type_error &) {
		Output("Loading error in '%s'\n", typeid(f).name());
		f->d.madeWithFactory = true;
		throw SavedGameCorruptException();
	}

	SfxManager::FromJson(frameObj, f->m_thisId);

	f->ClearMovement();
	return f->GetId();
}

void Frame::DeleteFrames()
{
	// for each set "madeWithFactory"...
	std::for_each(begin(s_frames), end(s_frames), [](Frame &f) {
		f.d.madeWithFactory = true;
	});
	// then delete it
	s_frames.clear();

	// remember to delete CollisionSpaces
	s_collisionSpaces.clear();
}

Frame *Frame::GetFrame(FrameId fId)
{
#ifndef NDEBUG
	if (fId && fId.id() >= s_frames.size())
		Error("In '%s': fId is valid but out of range (%zu)...\n", __func__, fId.id());
#endif

	return fId.valid() ? &s_frames[fId] : nullptr;
}

FrameId Frame::CreateCameraFrame(FrameId parent)
{
	Dummy dummy;
	dummy.madeWithFactory = true;

	s_frames.emplace_back(dummy, parent);
	return (s_frames.size() - 1);
}

void Frame::DeleteCameraFrame(FrameId camera)
{
	if (!camera)
		return;

	// Detach camera from parent, then delete:
	Frame *cameraFrame = Frame::GetFrame(camera);
	Frame *parent = Frame::GetFrame(cameraFrame->GetParent());
	if (parent)
		parent->RemoveChild(camera);

// Call dtor "popping" element in vector
#ifndef NDEBUG
	if (s_frames.size() > 0 && camera.id() < s_frames.size() - 1) {
		Error("DeleteCameraFrame: seems camera frame is not the last frame!\n");
		abort();
	};
#endif // NDEBUG
	s_frames.back().d.madeWithFactory = true;
	s_frames.pop_back();
}

void Frame::PostUnserializeFixup(FrameId fId, Space *space)
{
	Frame *f = Frame::GetFrame(fId);
	f->UpdateRootRelativeVars();
	f->m_astroBody = space->GetBodyByIndex(f->m_astroBodyIndex);
	// build the object trees once after loading so they're initialized while paused.
	f->GetCollisionSpace()->RebuildObjectTrees();
	for (FrameId kid : f->GetChildren())
		PostUnserializeFixup(kid, space);
}

void Frame::CollideFrames(void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()

	for (auto &frame : s_frames) {
		if (!frame.m_collisionSpace)
			continue;

		PROFILE_SCOPED_DESC(frame.m_label.c_str())
		frame.m_collisionSpace->Collide(callback);
	}
}

void Frame::RemoveChild(FrameId fId)
{
	PROFILE_SCOPED()
	if (!fId.valid()) return;
	Frame *f = Frame::GetFrame(fId);
	if (f == nullptr) return;
	const std::vector<FrameId>::iterator it = std::find(m_children.begin(), m_children.end(), fId);
	if (it != m_children.end())
		m_children.erase(it);
}

void Frame::AddGeom(Geom *g) { m_collisionSpace->AddGeom(g); }
void Frame::RemoveGeom(Geom *g) { m_collisionSpace->RemoveGeom(g); }
void Frame::AddStaticGeom(Geom *g) { m_collisionSpace->AddStaticGeom(g); }
void Frame::RemoveStaticGeom(Geom *g) { m_collisionSpace->RemoveStaticGeom(g); }
void Frame::SetPlanetGeom(double radius, Body *obj)
{
	m_collisionSpace->SetSphere(vector3d(0, 0, 0), radius, static_cast<void *>(obj));
}

CollisionSpace *Frame::GetCollisionSpace() const
{
	return m_collisionSpace.get();
}

// doesn't consider stasis velocity
vector3d Frame::GetVelocityRelTo(FrameId relToId) const
{
	if (m_thisId == relToId) return vector3d(0, 0, 0); // early-out to avoid unnecessary computation

	const Frame *relTo = Frame::GetFrame(relToId);
	vector3d diff = m_rootVel - relTo->m_rootVel;
	if (relTo->IsRotFrame())
		return diff * relTo->m_rootOrient;
	else
		return diff;
}

vector3d Frame::GetPositionRelTo(FrameId relToId) const
{
	// early-outs for simple cases, required for accuracy in large systems
	if (m_thisId == relToId) return vector3d(0, 0, 0);

	const Frame *relTo = Frame::GetFrame(relToId);

	if (GetParent() == relToId) return m_pos; // relative to parent
	if (relTo->GetParent() == m_thisId) {	  // relative to child
		if (!relTo->IsRotFrame())
			return -relTo->m_pos;
		else
			return -relTo->m_pos * relTo->m_orient;
	}
	if (relTo->GetParent() == GetParent()) { // common parent
		if (!relTo->IsRotFrame())
			return m_pos - relTo->m_pos;
		else
			return (m_pos - relTo->m_pos) * relTo->m_orient;
	}

	// use precalculated absolute position and orient
	vector3d diff = m_rootPos - relTo->m_rootPos;
	if (relTo->IsRotFrame())
		return diff * relTo->m_rootOrient;
	else
		return diff;
}

vector3d Frame::GetInterpPositionRelTo(FrameId relToId) const
{
	const Frame *relTo = Frame::GetFrame(relToId);

	// early-outs for simple cases, required for accuracy in large systems
	if (m_thisId == relToId) return vector3d(0, 0, 0);
	if (GetParent() == relTo->GetId()) return m_interpPos; // relative to parent
	if (relTo->GetParent() == m_thisId) {				   // relative to child
		if (!relTo->IsRotFrame())
			return -relTo->m_interpPos;
		else
			return -relTo->m_interpPos * relTo->m_interpOrient;
	}
	if (relTo->GetParent() == GetParent()) { // common parent
		if (!relTo->IsRotFrame())
			return m_interpPos - relTo->m_interpPos;
		else
			return (m_interpPos - relTo->m_interpPos) * relTo->m_interpOrient;
	}

	vector3d diff = m_rootInterpPos - relTo->m_rootInterpPos;
	if (relTo->IsRotFrame())
		return diff * relTo->m_rootInterpOrient;
	else
		return diff;
}

matrix3x3d Frame::GetOrientRelTo(FrameId relToId) const
{
	if (m_thisId == relToId) return matrix3x3d::Identity();
	return Frame::GetFrame(relToId)->m_rootOrient.Transpose() * m_rootOrient;
}

matrix3x3d Frame::GetInterpOrientRelTo(FrameId relToId) const
{
	if (m_thisId == relToId) return matrix3x3d::Identity();
	return Frame::GetFrame(relToId)->m_rootInterpOrient.Transpose() * m_rootInterpOrient;
	/*	if (IsRotFrame()) {
		if (relTo->IsRotFrame()) return m_interpOrient * relTo->m_interpOrient.Transpose();
		else return m_interpOrient;
	}
	if (relTo->IsRotFrame()) return relTo->m_interpOrient.Transpose();
	else return matrix3x3d::Identity();
*/
}

matrix4x4d Frame::GetTransformRelTo(FrameId relToId) const
{
	return matrix4x4d(GetOrientRelTo(relToId), GetPositionRelTo(relToId));
}

matrix4x4d Frame::GetInterpTransformRelTo(FrameId relToId) const
{
	return matrix4x4d(GetInterpOrientRelTo(relToId), GetInterpPositionRelTo(relToId));
}

void Frame::UpdateInterpTransform(double alpha)
{
	PROFILE_SCOPED()
	m_interpPos = alpha * m_pos + (1.0 - alpha) * m_oldPos;

	double len = m_oldAngDisplacement * (1.0 - alpha);
	if (!is_zero_exact(len)) {					   // very small values are normal here
		matrix3x3d rot = matrix3x3d::RotateY(len); // RotateY is backwards
		m_interpOrient = m_orient * rot;
	} else
		m_interpOrient = m_orient;

	Frame *parent = Frame::GetFrame(m_parent);
	if (!parent)
		ClearMovement();
	else {
		m_rootInterpPos = parent->m_rootInterpOrient * m_interpPos + parent->m_rootInterpPos;
		m_rootInterpOrient = parent->m_rootInterpOrient * m_interpOrient;
	}

	for (FrameId kid : m_children) {
		Frame *kidFrame = Frame::GetFrame(kid);
		kidFrame->UpdateInterpTransform(alpha);
	}
}

void Frame::GetFrameTransform(const FrameId fFromId, const FrameId fToId, matrix4x4d &m)
{
	matrix3x3d forient = Frame::GetFrame(fFromId)->GetOrientRelTo(fToId);
	vector3d fpos = Frame::GetFrame(fFromId)->GetPositionRelTo(fToId);
	m = forient;
	m.SetTranslate(fpos);
}

void Frame::ClearMovement()
{
	UpdateRootRelativeVars();
	m_rootInterpPos = m_rootPos;
	m_rootInterpOrient = m_rootOrient;
	m_oldPos = m_interpPos = m_pos;
	m_interpOrient = m_orient;
	m_oldAngDisplacement = 0.0;
}

void Frame::UpdateOrbitRails(double time, double timestep)
{
	PROFILE_SCOPED()
	std::for_each(begin(s_frames), end(s_frames), [&time, &timestep](Frame &frame) {
		frame.m_oldPos = frame.m_pos;
		frame.m_oldAngDisplacement = frame.m_angSpeed * timestep;

		// update frame position and velocity
		if (frame.m_parent.valid() && frame.m_sbody && !frame.IsRotFrame()) {
			frame.m_pos = frame.m_sbody->GetOrbit().OrbitalPosAtTime(time);
			vector3d pos2 = frame.m_sbody->GetOrbit().OrbitalPosAtTime(time + timestep);
			frame.m_vel = (pos2 - frame.m_pos) / timestep;
		}
		// temporary test thing
		else
			frame.m_pos = frame.m_pos + frame.m_vel * timestep;

		// update frame rotation
		double ang = fmod(frame.m_angSpeed * time, 2.0 * M_PI);
		if (!is_zero_exact(ang)) {						  // frequently used with e^-10 etc
			matrix3x3d rot = matrix3x3d::RotateY(-ang);	  // RotateY is backwards
			frame.m_orient = frame.m_initialOrient * rot; // angvel always +y
		}
		frame.UpdateRootRelativeVars(); // update root-relative pos/vel/orient
	});
	/*
	for (FrameId kid : m_children) {
		Frame *kidFrame = Frame::GetFrame(kid);
		kidFrame->UpdateOrbitRails(time, timestep);
	}
	*/
}

void Frame::SetInitialOrient(const matrix3x3d &m, double time)
{
	m_initialOrient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) {						// frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(-ang); // RotateY is backwards
		m_orient = m_initialOrient * rot;			// angvel always +y
	} else {
		m_orient = m_initialOrient;
	}
}

void Frame::SetOrient(const matrix3x3d &m, double time)
{
	m_orient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) {					   // frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(ang); // RotateY is backwards
		m_initialOrient = m_orient * rot;		   // angvel always +y
	} else {
		m_initialOrient = m_orient;
	}
}

void Frame::UpdateRootRelativeVars()
{
	// update pos & vel relative to parent frame
	Frame *parent = Frame::GetFrame(m_parent);
	if (!parent) {
		m_rootPos = m_rootVel = vector3d(0, 0, 0);
		m_rootOrient = matrix3x3d::Identity();
	} else {
		m_rootPos = parent->m_rootOrient * m_pos + parent->m_rootPos;
		m_rootVel = parent->m_rootOrient * m_vel + parent->m_rootVel;
		m_rootOrient = parent->m_rootOrient * m_orient;
	}
}
