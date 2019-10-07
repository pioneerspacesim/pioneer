// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Frame.h"

#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Sfx.h"
#include "Space.h"
#include "collider/CollisionSpace.h"
#include "utils.h"

std::list<Frame> Frame::s_frames;

Frame::Frame(const Dummy &d, FrameId parent, const char *label, unsigned int flags, double radius):
	m_sbody(nullptr),
	m_astroBody(nullptr),
	m_parent(parent),
	m_flags(flags),
	m_radius(radius),
	m_pos(vector3d(0.0)),
	m_vel(vector3d(0.0)),
	m_angSpeed(0.0),
	m_orient(matrix3x3d::Identity()),
	m_initialOrient(matrix3x3d::Identity())
{
	if (!d.madeWithFactory)
		Error("Frame ctor called directly!\n");

	m_thisId = s_frames.size();

	ClearMovement();
	m_collisionSpace = new CollisionSpace();
	if (IsIdValid(m_parent)) Frame::GetFrame(m_parent)->AddChild(m_thisId);
	if (label) m_label = label;
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
		Error("Frame instance deletion outside 'DeleteFrame'\n");
	}
	// Delete this Frame and recurse deleting children
	delete m_collisionSpace;
	for (FrameId kid : m_children) {
		DeleteFrame(kid);
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
	s_frames.emplace_back(dummy, noFrameId, nullptr);

	Frame *f = &s_frames.back();

	if (parent != noFrameId)
		f->m_parent = Frame::GetFrame(parent)->GetId();
	else
		f->m_parent = noFrameId;

	f->d.madeWithFactory = false;

	try {
		f->m_thisId = frameObj["frameId"];

		f->m_flags = frameObj["flags"];
		f->m_radius = frameObj["radius"];
		f->m_label = frameObj["label"];

		// Check if frames order in load and save are the same
		assert((s_frames.size() - 1) != f->m_thisId);

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
				FrameId kidId = FromJson(childFrameArray[i], space, f->m_thisId, at_time);
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

void Frame::DeleteFrame(FrameId tobedeleted)
{
	Frame *f = GetFrame(tobedeleted);
	f->d.madeWithFactory = true;
	// Find Frame and delete it, let dtor delete its children
	for (std::list<Frame>::const_iterator it = s_frames.begin(); it != s_frames.end(); it++) {
		if (f == &(*it)) {
			s_frames.erase(it);
			break;
		}
	}
}

Frame *Frame::GetFrame(FrameId FId)
{
	for (Frame &elem : s_frames) {
		if (elem.m_thisId == FId) return &elem;
	}
	return nullptr;
}

void Frame::PostUnserializeFixup(FrameId fId, Space *space)
{
	Frame *f = Frame::GetFrame(fId);
	f->UpdateRootRelativeVars();
	f->m_astroBody = space->GetBodyByIndex(f->m_astroBodyIndex);
	for (FrameId kid : f->GetChildren())
		PostUnserializeFixup(kid, space);
}

void Frame::RemoveChild(FrameId fId)
{
	PROFILE_SCOPED()
	if (fId == noFrameId) return;
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
	if (relTo->GetParent() == m_thisId) { // relative to child
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
	if (relTo->GetParent() == m_thisId) { // relative to child
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

void Frame::UpdateInterpTransform(double alpha)
{
	PROFILE_SCOPED()
	m_interpPos = alpha * m_pos + (1.0 - alpha) * m_oldPos;

	double len = m_oldAngDisplacement * (1.0 - alpha);
	if (!is_zero_exact(len)) { // very small values are normal here
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
	m_oldPos = m_pos;
	m_oldAngDisplacement = m_angSpeed * timestep;

	// update frame position and velocity
	if (IsIdValid(m_parent) && m_sbody && !IsRotFrame()) {
		m_pos = m_sbody->GetOrbit().OrbitalPosAtTime(time);
		vector3d pos2 = m_sbody->GetOrbit().OrbitalPosAtTime(time + timestep);
		m_vel = (pos2 - m_pos) / timestep;
	}
	// temporary test thing
	else
		m_pos = m_pos + m_vel * timestep;

	// update frame rotation
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) { // frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(-ang); // RotateY is backwards
		m_orient = m_initialOrient * rot; // angvel always +y
	}
	UpdateRootRelativeVars(); // update root-relative pos/vel/orient

	for (FrameId kid : m_children) {
		Frame *kidFrame = Frame::GetFrame(kid);
		kidFrame->UpdateOrbitRails(time, timestep);
	}
}

void Frame::SetInitialOrient(const matrix3x3d &m, double time)
{
	m_initialOrient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) { // frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(-ang); // RotateY is backwards
		m_orient = m_initialOrient * rot; // angvel always +y
	} else {
		m_orient = m_initialOrient;
	}
}

void Frame::SetOrient(const matrix3x3d &m, double time)
{
	m_orient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) { // frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(ang); // RotateY is backwards
		m_initialOrient = m_orient * rot; // angvel always +y
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
