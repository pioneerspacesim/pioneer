// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Frame.h"
#include "Body.h"
#include "Space.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "galaxy/StarSystem.h"
#include "Pi.h"
#include "Game.h"
#include "json/JsonUtils.h"
#include <algorithm>

Frame::Frame()
{
	Init(0, "", FLAG_DEFAULT);
}

Frame::Frame(Frame *parent, const char *label)
{
	Init(parent, label, FLAG_DEFAULT);
}

Frame::Frame(Frame *parent, const char *label, unsigned int flags)
{
	Init(parent, label, flags);
}

void Frame::ToJson(Json::Value &jsonObj, Frame *f, Space *space)
{
	Json::Value frameObj(Json::objectValue); // Create JSON object to contain frame data.

	frameObj["flags"] = f->m_flags;
	frameObj["radius"] = DoubleToStr(f->m_radius);
	frameObj["label"] = f->m_label;
	VectorToJson(frameObj, f->m_pos, "pos");
	frameObj["ang_speed"] = DoubleToStr(f->m_angSpeed);
	MatrixToJson(frameObj, f->m_initialOrient, "init_orient");
	frameObj["index_for_system_body"] = space->GetIndexForSystemBody(f->m_sbody);
	frameObj["index_for_astro_body"] = space->GetIndexForBody(f->m_astroBody);

	Json::Value childFrameArray(Json::arrayValue); // Create JSON array to contain child frame data.
	for (Frame* kid : f->GetChildren())
	{
		Json::Value childFrameArrayEl(Json::objectValue); // Create JSON object to contain child frame.
		Frame::ToJson(childFrameArrayEl, kid, space);
		childFrameArray.append(childFrameArrayEl); // Append child frame object to array.
	}
	frameObj["child_frames"] = childFrameArray; // Add child frame array to frame object.

	SfxManager::ToJson(frameObj, f);

	jsonObj["frame"] = frameObj; // Add frame object to supplied object.
}

Frame *Frame::FromJson(const Json::Value &jsonObj, Space *space, Frame *parent, double at_time)
{
	Frame *f = new Frame();
	f->m_parent = parent;

	if (!jsonObj.isMember("frame")) throw SavedGameCorruptException();
	Json::Value frameObj = jsonObj["frame"];

	if (!frameObj.isMember("flags")) throw SavedGameCorruptException();
	if (!frameObj.isMember("radius")) throw SavedGameCorruptException();
	if (!frameObj.isMember("label")) throw SavedGameCorruptException();
	if (!frameObj.isMember("pos")) throw SavedGameCorruptException();
	if (!frameObj.isMember("ang_speed")) throw SavedGameCorruptException();
	if (!frameObj.isMember("init_orient")) throw SavedGameCorruptException();
	if (!frameObj.isMember("index_for_system_body")) throw SavedGameCorruptException();
	if (!frameObj.isMember("index_for_astro_body")) throw SavedGameCorruptException();

	f->m_flags = frameObj["flags"].asInt();
	f->m_radius = StrToDouble(frameObj["radius"].asString());
	f->m_label = frameObj["label"].asString();
	JsonToVector(&(f->m_pos), frameObj, "pos");
	f->m_angSpeed = StrToDouble(frameObj["ang_speed"].asString());
	matrix3x3d orient;
	JsonToMatrix(&orient, frameObj, "init_orient");
	f->SetInitialOrient(orient, at_time);
	f->m_sbody = space->GetSystemBodyByIndex(frameObj["index_for_system_body"].asUInt());
	f->m_astroBodyIndex = frameObj["index_for_astro_body"].asUInt();
	f->m_vel = vector3d(0.0); // m_vel is set to zero.

	if (!frameObj.isMember("child_frames")) throw SavedGameCorruptException();
	Json::Value childFrameArray = frameObj["child_frames"];
	if (!childFrameArray.isArray()) throw SavedGameCorruptException();
	for (unsigned int i = 0; i < childFrameArray.size(); ++i) {
		f->m_children.push_back(FromJson(childFrameArray[i], space, f, at_time));
	}
	SfxManager::FromJson(frameObj, f);

	f->ClearMovement();
	return f;
}

void Frame::PostUnserializeFixup(Frame *f, Space *space)
{
	f->UpdateRootRelativeVars();
	f->m_astroBody = space->GetBodyByIndex(f->m_astroBodyIndex);
	for (Frame* kid : f->GetChildren())
		PostUnserializeFixup(kid, space);
}

void Frame::Init(Frame *parent, const char *label, unsigned int flags)
{
	m_sfx = 0;
	m_sbody = 0;
	m_astroBody = 0;
	m_parent = parent;
	m_flags = flags;
	m_radius = 0;
	m_pos = vector3d(0.0);
	m_vel = vector3d(0.0);
	m_angSpeed = 0.0;
	m_orient = matrix3x3d::Identity();
	m_initialOrient = matrix3x3d::Identity();
	ClearMovement();
	m_collisionSpace = new CollisionSpace();
	if (m_parent) m_parent->AddChild(this);
	if (label) m_label = label;
	m_sbody = nullptr;
}

Frame::~Frame()
{
	m_sfx.reset();
	delete m_collisionSpace;
	for (Frame* kid : m_children)
		delete kid;
}

void Frame::RemoveChild(Frame *f)
{
	PROFILE_SCOPED()
	const std::vector<Frame*>::iterator it
		= std::find(m_children.begin(), m_children.end(), f);
	if (it != m_children.end())
		m_children.erase(it);
}

void Frame::AddGeom(Geom *g) { m_collisionSpace->AddGeom(g); }
void Frame::RemoveGeom(Geom *g) { m_collisionSpace->RemoveGeom(g); }
void Frame::AddStaticGeom(Geom *g) { m_collisionSpace->AddStaticGeom(g); }
void Frame::RemoveStaticGeom(Geom *g) { m_collisionSpace->RemoveStaticGeom(g); }
void Frame::SetPlanetGeom(double radius, Body *obj)
{
	m_collisionSpace->SetSphere(vector3d(0,0,0), radius, static_cast<void*>(obj));
}

// doesn't consider stasis velocity
vector3d Frame::GetVelocityRelTo(const Frame *relTo) const
{
	if (this == relTo) return vector3d(0,0,0); // early-out to avoid unnecessary computation
	vector3d diff = m_rootVel - relTo->m_rootVel;
	if (relTo->IsRotFrame()) return diff * relTo->m_rootOrient;
	else return diff;
}

vector3d Frame::GetPositionRelTo(const Frame *relTo) const
{
	// early-outs for simple cases (disabled as premature optimisation)
//	if (this == relTo) return vector3d(0,0,0);
//	if (relTo->GetParent() == this) 					// relative to child
//		return -relTo->m_pos * relTo->m_orient;
//	if (GetParent() == relTo) return m_pos;				// relative to parent

	vector3d diff = m_rootPos - relTo->m_rootPos;
	if (relTo->IsRotFrame()) return diff * relTo->m_rootOrient;
	else return diff;
}

vector3d Frame::GetInterpPositionRelTo(const Frame *relTo) const
{
	// early-outs for simple cases (disabled as premature optimisation)
//	if (this == relTo) return vector3d(0,0,0);
//	if (relTo->GetParent() == this) 							// relative to child
//		return -relTo->m_interpPos * relTo->m_interpOrient;
//	if (GetParent() == relTo) return m_interpPos;				// relative to parent

	vector3d diff = m_rootInterpPos - relTo->m_rootInterpPos;
	if (relTo->IsRotFrame()) return diff * relTo->m_rootInterpOrient;
	else return diff;
}

matrix3x3d Frame::GetOrientRelTo(const Frame *relTo) const
{
	if (this == relTo) return matrix3x3d::Identity();
	return relTo->m_rootOrient.Transpose() * m_rootOrient;
}

matrix3x3d Frame::GetInterpOrientRelTo(const Frame *relTo) const
{
	if (this == relTo) return matrix3x3d::Identity();
	return relTo->m_rootInterpOrient.Transpose() * m_rootInterpOrient;
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
	m_interpPos = alpha*m_pos + (1.0-alpha)*m_oldPos;

	double len = m_oldAngDisplacement * (1.0-alpha);
	if (!is_zero_exact(len)) {			// very small values are normal here
		matrix3x3d rot = matrix3x3d::RotateY(len);		// RotateY is backwards
		m_interpOrient = m_orient * rot;
	}
	else m_interpOrient = m_orient;

	if (!m_parent) ClearMovement();
	else {
		m_rootInterpPos = m_parent->m_rootInterpOrient * m_interpPos
			+ m_parent->m_rootInterpPos;
		m_rootInterpOrient = m_parent->m_rootInterpOrient * m_interpOrient;
	}

	for (Frame* kid : m_children)
		kid->UpdateInterpTransform(alpha);
}

void Frame::GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m)
{
	matrix3x3d forient = fFrom->GetOrientRelTo(fTo);
	vector3d fpos = fFrom->GetPositionRelTo(fTo);
	m = forient; m.SetTranslate(fpos);
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
	if (m_parent && m_sbody && !IsRotFrame()) {
		m_pos = m_sbody->GetOrbit().OrbitalPosAtTime(time);
		vector3d pos2 = m_sbody->GetOrbit().OrbitalPosAtTime(time+timestep);
		m_vel = (pos2 - m_pos) / timestep;
	}
	// temporary test thing
	else m_pos = m_pos + m_vel * timestep;
	
	// update frame rotation
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) {			// frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(-ang);		// RotateY is backwards
		m_orient = m_initialOrient * rot;		// angvel always +y
	}
	UpdateRootRelativeVars();			// update root-relative pos/vel/orient

	for (Frame* kid : m_children)
		kid->UpdateOrbitRails(time, timestep);
}

void Frame::SetInitialOrient(const matrix3x3d &m, double time) {
	m_initialOrient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) {			// frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(-ang);		// RotateY is backwards
		m_orient = m_initialOrient * rot;		// angvel always +y
	} else {
		m_orient = m_initialOrient;
	}
}

void Frame::SetOrient(const matrix3x3d &m, double time) {
	m_orient = m;
	double ang = fmod(m_angSpeed * time, 2.0 * M_PI);
	if (!is_zero_exact(ang)) {			// frequently used with e^-10 etc
		matrix3x3d rot = matrix3x3d::RotateY(ang);		// RotateY is backwards
		m_initialOrient = m_orient * rot;		// angvel always +y
	} else {
		m_initialOrient = m_orient;
	}
}

void Frame::UpdateRootRelativeVars()
{
	// update pos & vel relative to parent frame
	if (!m_parent) {
		m_rootPos = m_rootVel = vector3d(0,0,0);
		m_rootOrient = matrix3x3d::Identity();
	}
	else {
		m_rootPos = m_parent->m_rootOrient * m_pos + m_parent->m_rootPos;
		m_rootVel = m_parent->m_rootOrient * m_vel + m_parent->m_rootVel;
		m_rootOrient = m_parent->m_rootOrient * m_orient;
	}
}
