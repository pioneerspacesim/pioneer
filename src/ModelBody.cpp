// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelBody.h"

#include "Camera.h"
#include "Frame.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Pi.h"
#include "Planet.h"
#include "Shields.h"
#include "collider/CollisionSpace.h"
#include "collider/Geom.h"
#include "galaxy/SystemBody.h"
#include "scenegraph/Animation.h"
#include "scenegraph/CollisionGeometry.h"
#include "scenegraph/NodeVisitor.h"
#include "scenegraph/SceneGraph.h"

class Space;

class DynGeomFinder : public SceneGraph::NodeVisitor {
public:
	std::vector<SceneGraph::CollisionGeometry *> results;

	virtual void ApplyCollisionGeometry(SceneGraph::CollisionGeometry &cg)
	{
		if (cg.IsDynamic())
			results.push_back(&cg);
	}

	SceneGraph::CollisionGeometry *GetCgForTree(GeomTree *t)
	{
		for (auto it = results.begin(); it != results.end(); ++it)
			if ((*it)->GetGeomTree() == t) return (*it);
		return 0;
	}
};

class DynCollUpdateVisitor : public SceneGraph::NodeVisitor {
private:
	std::vector<matrix4x4f> m_matrixStack;

public:
	void Reset() { m_matrixStack.clear(); }

	virtual void ApplyMatrixTransform(SceneGraph::MatrixTransform &m)
	{
		matrix4x4f matrix = matrix4x4f::Identity();
		if (!m_matrixStack.empty()) matrix = m_matrixStack.back();

		m_matrixStack.push_back(matrix * m.GetTransform());
		m.Traverse(*this);
		m_matrixStack.pop_back();
	}

	virtual void ApplyCollisionGeometry(SceneGraph::CollisionGeometry &cg)
	{
		if (!cg.GetGeom()) return;

		cg.GetGeom()->m_animTransform = matrix4x4d(m_matrixStack.back());
	}
};

ModelBody::ModelBody() :
	m_isStatic(false),
	m_colliding(true),
	m_geom(nullptr),
	m_model(nullptr)
{
}

ModelBody::ModelBody(const Json &jsonObj, Space *space) :
	Body(jsonObj, space),
	m_geom(nullptr),
	m_model(nullptr)
{
	Json modelBodyObj = jsonObj["model_body"];

	try {
		m_isStatic = modelBodyObj["is_static"];
		SetModel(modelBodyObj["model_name"].get<std::string>().c_str());
		SetColliding(modelBodyObj["is_colliding"]);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	m_model->LoadFromJson(modelBodyObj);
	m_shields->LoadFromJson(modelBodyObj);
}

ModelBody::~ModelBody()
{
	SetFrame(FrameId::Invalid); // Will remove geom from frame if necessary.
	DeleteGeoms();

	//delete instanced model
	delete m_model;
}

void ModelBody::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json modelBodyObj = Json::object(); // Create JSON object to contain model body data.

	modelBodyObj["is_static"] = m_isStatic;
	modelBodyObj["is_colliding"] = m_colliding;
	modelBodyObj["model_name"] = m_modelName;
	m_model->SaveToJson(modelBodyObj);
	m_shields->SaveToJson(modelBodyObj);

	jsonObj["model_body"] = modelBodyObj; // Add model body object to supplied object.
}

void ModelBody::SetStatic(bool isStatic)
{
	if (isStatic == m_isStatic) return;
	m_isStatic = isStatic;
	if (!m_geom) return;

	Frame *f = Frame::GetFrame(GetFrame());
	if (m_isStatic) {
		f->RemoveGeom(m_geom);
		f->AddStaticGeom(m_geom);
	} else {
		f->RemoveStaticGeom(m_geom);
		f->AddGeom(m_geom);
	}
}

void ModelBody::SetColliding(bool colliding)
{
	m_colliding = colliding;
	if (colliding) {
		m_geom->Enable();
		for(auto &g : m_dynGeoms) {
			g->Enable();
		}
	} else {
		m_geom->Disable();
		for(auto &g : m_dynGeoms) {
			g->Disable();
		}
	}
}

void ModelBody::RebuildCollisionMesh()
{
	Frame *f = Frame::GetFrame(GetFrame());
	if (m_geom) {
		if (f) RemoveGeomsFromFrame(f);
		DeleteGeoms();
	}

	m_collMesh = m_model->GetCollisionMesh();
	double maxRadius = m_collMesh->GetAabb().GetRadius();

	//static geom
	m_geom = new Geom(m_collMesh->GetGeomTree(), GetOrient(), GetPosition(), this);
	if(!IsColliding()) m_geom->Disable();

	SetPhysRadius(maxRadius);

	//have to figure out which collision geometries are responsible for which geomtrees
	DynGeomFinder dgf;
	m_model->GetRoot()->Accept(dgf);

	//dynamic geoms
	for (auto it = m_collMesh->GetDynGeomTrees().begin(); it != m_collMesh->GetDynGeomTrees().end(); ++it) {
		Geom *dynG = new Geom(*it, GetOrient(), GetPosition(), this);
		dynG->m_animTransform = matrix4x4d::Identity();
		SceneGraph::CollisionGeometry *cg = dgf.GetCgForTree(*it);
		if (cg)
			cg->SetGeom(dynG);
		if(!IsColliding()) dynG->Disable();
		m_dynGeoms.push_back(dynG);
	}

	if (f) AddGeomsToFrame(f);
}

void ModelBody::SetModel(const char *modelName)
{
	//remove old instance
	delete m_model;
	m_model = 0;

	m_modelName = modelName;

	//create model instance (some modelbodies, like missiles could avoid this)
	m_model = Pi::FindModel(m_modelName)->MakeInstance();
	m_idleAnimation = m_model->FindAnimation("idle");
	// TODO: this isn't great, as animations will be ticked regardless of whether the modelbody
	// is next to the player or on the other side of the solar system.
	if (m_idleAnimation)
		m_model->SetAnimationActive(m_model->FindAnimationIndex(m_idleAnimation), true);

	SetClipRadius(m_model->GetDrawClipRadius());

	m_shields.reset(new Shields(m_model));

	RebuildCollisionMesh();
}

void ModelBody::SetPosition(const vector3d &p)
{
	Body::SetPosition(p);
	MoveGeoms(GetOrient(), p);
}

void ModelBody::SetOrient(const matrix3x3d &m)
{
	Body::SetOrient(m);
	const matrix4x4d m2 = m;
	MoveGeoms(m2, GetPosition());
}

void ModelBody::SetFrame(FrameId fId)
{
	if (fId == GetFrame()) return;

	//remove collision geoms from old frame
	Frame *f = Frame::GetFrame(GetFrame());
	if (f) RemoveGeomsFromFrame(f);

	Body::SetFrame(fId);

	//add collision geoms to new frame
	f = Frame::GetFrame(GetFrame());
	if (f) AddGeomsToFrame(f);
}

void ModelBody::DeleteGeoms()
{
	delete m_geom;
	m_geom = nullptr;
	for (auto it = m_dynGeoms.begin(); it != m_dynGeoms.end(); ++it)
		delete *it;
	m_dynGeoms.clear();
}

void ModelBody::AddGeomsToFrame(Frame *f)
{
	const int group = f->GetCollisionSpace()->GetGroupHandle();

	m_geom->SetGroup(group);

	if (m_isStatic) {
		f->AddStaticGeom(m_geom);
	} else {
		f->AddGeom(m_geom);
	}

	for (auto it = m_dynGeoms.begin(); it != m_dynGeoms.end(); ++it) {
		(*it)->SetGroup(group);
		f->AddGeom(*it);
	}
}

void ModelBody::RemoveGeomsFromFrame(Frame *f)
{
	if (f == nullptr) return;

	if (m_isStatic) {
		f->RemoveStaticGeom(m_geom);
	} else {
		f->RemoveGeom(m_geom);
	}

	for (auto it = m_dynGeoms.begin(); it != m_dynGeoms.end(); ++it)
		f->RemoveGeom(*it);
}

void ModelBody::MoveGeoms(const matrix4x4d &m, const vector3d &p)
{
	m_geom->MoveTo(m, p);

	//accumulate transforms to animated positions
	if (!m_dynGeoms.empty()) {
		DynCollUpdateVisitor dcv;
		m_model->GetRoot()->Accept(dcv);
	}

	for (auto it = m_dynGeoms.begin(); it != m_dynGeoms.end(); ++it) {
		//combine orient & pos
		static matrix4x4d s_tempMat;
		for (unsigned int i = 0; i < 12; i++)
			s_tempMat[i] = m[i];
		s_tempMat[12] = p.x;
		s_tempMat[13] = p.y;
		s_tempMat[14] = p.z;
		s_tempMat[15] = m[15];

		(*it)->MoveTo(s_tempMat * (*it)->m_animTransform);
	}
}

void ModelBody::RenderModel(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d m2 = GetInterpOrient();
	m2.SetTranslate(GetInterpPosition());

	m_model->Render(matrix4x4f(viewTransform * m2));
}

void ModelBody::TimeStepUpdate(const float timestep)
{
	if (m_idleAnimation)
		// step animation by timestep/total length, loop to 0.0 if it goes >= 1.0
		m_idleAnimation->SetProgress(fmod(m_idleAnimation->GetProgress() + timestep / m_idleAnimation->GetDuration(), 1.0));

	m_model->UpdateAnimations();
}
