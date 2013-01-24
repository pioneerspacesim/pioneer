// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "ModelBody.h"
#include "collider/collider.h"
#include "Frame.h"
#include "Game.h"
#include "graphics/Renderer.h"
#include "LmrModel.h"
#include "matrix4x4.h"
#include "Pi.h"
#include "Serializer.h"
#include "Space.h"
#include "WorldView.h"
#include "ModelCache.h"
#include "scenegraph/Model.h"

ModelBody::ModelBody() :
	Body(),
	m_isStatic(false),
	m_colliding(true),
	m_geom(0),
	m_model(0)
{
	memset(&m_params, 0, sizeof(LmrObjParams));
}

ModelBody::~ModelBody()
{
	SetFrame(0);	// Will remove geom from frame if necessary.
	if (m_geom) delete m_geom;
}

void ModelBody::Save(Serializer::Writer &wr, Space *space)
{
	Body::Save(wr, space);
	wr.Bool(m_isStatic);
	wr.Bool(m_colliding);
}

void ModelBody::Load(Serializer::Reader &rd, Space *space)
{
	Body::Load(rd, space);
	m_isStatic = rd.Bool();
	m_colliding = rd.Bool();
}

void ModelBody::SetStatic(bool isStatic)
{
	if (isStatic == m_isStatic) return;
	m_isStatic = isStatic;
	if (!m_geom) return;

	if (m_isStatic) {
		GetFrame()->RemoveGeom(m_geom);
		GetFrame()->AddStaticGeom(m_geom);
	}
	else {
		GetFrame()->RemoveStaticGeom(m_geom);
		GetFrame()->AddGeom(m_geom);
	}
}

void ModelBody::SetColliding(bool colliding)
{
	m_colliding = colliding;
	if (!m_geom) return;

	if(colliding) m_geom->Enable();
	else m_geom->Disable();
}

void ModelBody::RebuildCollisionMesh()
{
	if (m_geom) {
		// only happens when player changes their ship
		if (m_isStatic) GetFrame()->RemoveStaticGeom(m_geom);
		else GetFrame()->RemoveGeom(m_geom);
		delete m_geom;
	}

	m_collMesh = m_model->CreateCollisionMesh(&m_params);
	SetPhysRadius(m_collMesh->GetAabb().GetRadius());
	m_geom = new Geom(m_collMesh->GetGeomTree());

	m_geom->SetUserData(static_cast<void*>(this));
	m_geom->MoveTo(GetOrient(), GetPosition());

	if (GetFrame()) {
		if (m_isStatic) GetFrame()->AddStaticGeom(m_geom);
		else GetFrame()->AddGeom(m_geom);
	}
}

void ModelBody::SetModel(const char *modelName)
{
	m_model = Pi::FindModel(modelName);
	SetClipRadius(m_model->GetDrawClipRadius());

	RebuildCollisionMesh();
}

void ModelBody::SetPosition(const vector3d &p)
{
	Body::SetPosition(p);
	if (!m_geom) return;
	matrix4x4d m2 = GetOrient();
	m_geom->MoveTo(m2, p);
	// for rebuild of static objects in collision space
	if (m_isStatic) SetFrame(GetFrame());
}

void ModelBody::SetOrient(const matrix3x3d &m)
{
	Body::SetOrient(m);
	if (!m_geom) return;
	matrix4x4d m2 = m;
	m_geom->MoveTo(m2, GetPosition());
}

void ModelBody::SetFrame(Frame *f)
{
	if (f == GetFrame()) return;
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

void ModelBody::RenderLmrModel(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d m2 = GetInterpOrient();
	m2.SetTranslate(GetInterpPosition());
	matrix4x4d t = viewTransform * m2;
	glPushMatrix();				// Otherwise newmodels leave a dirty matrix
	matrix4x4f trans;
	for (int i=0; i<12; i++) trans[i] = float(t[i]);
	trans[12] = viewCoords.x;
	trans[13] = viewCoords.y;
	trans[14] = viewCoords.z;
	trans[15] = 1.0f;

	m_params.label = GetLabel().c_str();
	m_model->Render(r, trans, &m_params);
	glPopMatrix();
}
