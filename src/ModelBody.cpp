// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "ModelBody.h"
#include "Space.h"
#include "Frame.h"
#include "Game.h"
#include "Pi.h"
#include "WorldView.h"
#include "Serializer.h"
#include "collider/collider.h"

ModelBody::ModelBody(): Body()
{
	m_lmrModel = 0;
	m_collMesh = 0;
	m_geom = 0;
	m_isStatic = false;
	memset(&m_params, 0, sizeof(LmrObjParams));
}

ModelBody::~ModelBody()
{
	SetFrame(0);	// Will remove geom from frame if necessary.
	if (m_collMesh) delete m_collMesh;
	delete m_geom;
}

void ModelBody::Save(Serializer::Writer &wr, Space *space)
{
	Body::Save(wr, space);
}

void ModelBody::Load(Serializer::Reader &rd, Space *space)
{
	Body::Load(rd, space);
}

void ModelBody::Disable()
{
	m_geom->Disable();
}

void ModelBody::Enable()
{
	m_geom->Enable();
}

const Aabb &ModelBody::GetAabb() const
{
	return m_collMesh->GetAabb();
}

void ModelBody::RebuildCollisionMesh()
{
	if (m_geom) {
		// only happens when player changes their ship
		if (m_isStatic) GetFrame()->RemoveStaticGeom(m_geom);
		else GetFrame()->RemoveGeom(m_geom);
		delete m_geom;
	}
	if (m_collMesh) delete m_collMesh;

	m_collMesh = new LmrCollMesh(m_lmrModel, &m_params);

	m_geom = new Geom(m_collMesh->geomTree);
	m_geom->SetUserData(static_cast<void*>(this));

	if (GetFrame()) {
		if (m_isStatic) GetFrame()->AddStaticGeom(m_geom);
		else GetFrame()->AddGeom(m_geom);
	}
}

void ModelBody::SetModel(const char *lmrModelName, bool isStatic)
{
	m_isStatic = isStatic;

	try {
		m_lmrModel = LmrLookupModelByName(lmrModelName);
	} catch (LmrModelNotFoundException) {
		printf("Could not find model '%s'.\n", lmrModelName);
		Pi::Quit();
	}

	RebuildCollisionMesh();
}

void ModelBody::SetPosition(const vector3d &p)
{
	Body::SetPosition(p);
	matrix4x4d m2 = GetOrient();
	m_geom->MoveTo(m2, p);
	// for rebuild of static objects in collision space
	if (m_isStatic) SetFrame(GetFrame());
}

void ModelBody::SetOrient(const matrix3x3d &m)
{
	Body::SetOrient(m);
	matrix4x4d m2 = m;
	m_geom->MoveTo(m2, GetPosition());
}

double ModelBody::GetBoundingRadius() const
{
	return m_lmrModel->GetDrawClipRadius();
}

void ModelBody::SetFrame(Frame *f)
{
	if (GetFrame()) {
		if (m_isStatic) GetFrame()->RemoveStaticGeom(m_geom);
		else GetFrame()->RemoveGeom(m_geom);
	}
	Body::SetFrame(f);
	if (f) {
		if (m_isStatic) f->AddStaticGeom(m_geom);
		else f->AddGeom(m_geom);
	}
}

void ModelBody::SetLmrTimeParams()
{
	m_params.time = Pi::game->GetTime();
}

void ModelBody::RenderLmrModel(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d m2 = GetInterpOrient();
	m2.SetTranslate(GetInterpPosition());
	matrix4x4d t = viewTransform * m2;
	matrix4x4f trans;
	for (int i=0; i<12; i++) trans[i] = float(t[i]);
	trans[12] = viewCoords.x;
	trans[13] = viewCoords.y;
	trans[14] = viewCoords.z;
	trans[15] = 1.0f;

	m_lmrModel->Render(trans, &m_params);
}
