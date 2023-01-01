// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelBody.h"

#include "Camera.h"
#include "Frame.h"
#include "GameSaveError.h"
#include "Json.h"
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
	if (colliding)
		m_geom->Enable();
	else
		m_geom->Disable();
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

// Calculates the ambiently and directly lit portions of the lighting model taking into account the atmosphere and sun positions at a given location
// 1. Calculates the amount of direct illumination available taking into account
//    * multiple suns
//    * sun positions relative to up direction i.e. light is dimmed as suns set
//    * Thickness of the atmosphere overhead i.e. as atmospheres get thicker light starts dimming earlier as sun sets, without atmosphere the light switches off at point of sunset
// 2. Calculates the split between ambient and directly lit portions taking into account
//    * Atmosphere density (optical thickness) of the sky dome overhead
//        as optical thickness increases the fraction of ambient light increases
//        this takes altitude into account automatically
//    * As suns set the split is biased towards ambient
void ModelBody::CalcLighting(double &ambient, double &direct, const Camera *camera)
{
	const double minAmbient = 0.05;
	ambient = minAmbient;
	direct = 1.0;
	Body *astro = Frame::GetFrame(GetFrame())->GetBody();
	if (!(astro && astro->IsType(ObjectType::PLANET)))
		return;

	Planet *planet = static_cast<Planet *>(astro);

	// position relative to the rotating frame of the planet
	vector3d upDir = GetInterpPositionRelTo(planet->GetFrame());
	const double planetRadius = planet->GetSystemBody()->GetRadius();
	const double dist = std::max(planetRadius, upDir.Length());
	upDir = upDir.Normalized();

	double pressure, density;
	planet->GetAtmosphericState(dist, &pressure, &density);
	double surfaceDensity;
	Color cl;
	planet->GetSystemBody()->GetAtmosphereFlavor(&cl, &surfaceDensity);

	// approximate optical thickness fraction as fraction of density remaining relative to earths
	double opticalThicknessFraction = density / EARTH_ATMOSPHERE_SURFACE_DENSITY;

	// tweak optical thickness curve - lower exponent ==> higher altitude before ambient level drops
	// Commenting this out, since it leads to a sharp transition at
	// atmosphereRadius, where density is suddenly 0
	//opticalThicknessFraction = pow(std::max(0.00001,opticalThicknessFraction),0.15); //max needed to avoid 0^power

	if (opticalThicknessFraction < 0.0001)
		return;

	//step through all the lights and calculate contributions taking into account sun position
	double light = 0.0;
	double light_clamped = 0.0;

	const std::vector<Camera::LightSource> &lightSources = camera->GetLightSources();
	for (std::vector<Camera::LightSource>::const_iterator l = lightSources.begin();
		 l != lightSources.end(); ++l) {
		double sunAngle;
		// calculate the extent the sun is towards zenith
		if (l->GetBody()) {
			// relative to the rotating frame of the planet
			const vector3d lightDir = (l->GetBody()->GetInterpPositionRelTo(planet->GetFrame()).Normalized());
			sunAngle = lightDir.Dot(upDir);
		} else {
			// light is the default light for systems without lights
			sunAngle = 1.0;
		}

		const double critAngle = -sqrt(dist * dist - planetRadius * planetRadius) / dist;

		//0 to 1 as sunangle goes from critAngle to 1.0
		double sunAngle2 = (Clamp(sunAngle, critAngle, 1.0) - critAngle) / (1.0 - critAngle);

		// angle at which light begins to fade on Earth
		const double surfaceStartAngle = 0.3;
		// angle at which sun set completes, which should be after sun has dipped below the horizon on Earth
		const double surfaceEndAngle = -0.18;

		const double start = std::min((surfaceStartAngle * opticalThicknessFraction), 1.0);
		const double end = std::max((surfaceEndAngle * opticalThicknessFraction), -0.2);

		sunAngle = (Clamp(sunAngle - critAngle, end, start) - end) / (start - end);

		light += sunAngle;
		light_clamped += sunAngle2;
	}

	light_clamped /= lightSources.size();
	light /= lightSources.size();

	// brightness depends on optical depth and intensity of light from all the stars
	direct = 1.0 - Clamp((1.0 - light), 0.0, 1.0) * Clamp(opticalThicknessFraction, 0.0, 1.0);

	// ambient light fraction
	// alter ratio between directly and ambiently lit portions towards ambiently lit as sun sets
	const double fraction = (0.2 + 0.8 * (1.0 - light_clamped)) * Clamp(opticalThicknessFraction, 0.0, 1.0);

	// fraction of light left over to be lit directly
	direct = (1.0 - fraction) * direct;

	// scale ambient by amount of light
	ambient = fraction * (Clamp((light), 0.0, 1.0)) * 0.25;

	ambient = std::max(minAmbient, ambient);
}

// setLighting: set renderer lights according to current position and sun
// positions. Original lighting is passed back in oldLights, oldAmbient, and
// should be reset after rendering with ModelBody::ResetLighting.
void ModelBody::SetLighting(Graphics::Renderer *r, const Camera *camera, std::vector<float> &oldIntensity, Color &oldAmbient)
{
	double ambient, direct;
	CalcLighting(ambient, direct, camera);

	const std::vector<Camera::LightSource> &lightSources = camera->GetLightSources();
	oldIntensity.resize(lightSources.size(), 0.0f);
	std::vector<float> lightIntensity(lightSources.size(), 0.0f);
	for (size_t i = 0; i < lightSources.size(); i++) {
		oldIntensity[i] = r->GetLight(i).GetIntensity();
		lightIntensity[i] = direct * camera->ShadowedIntensity(i, this);
	}

	oldAmbient = r->GetAmbientColor();
	r->SetAmbientColor(Color(ambient * 255, ambient * 255, ambient * 255));
	r->SetLightIntensity(lightSources.size(), lightIntensity.data());
}

void ModelBody::ResetLighting(Graphics::Renderer *r, const std::vector<float> &oldIntensity, const Color &oldAmbient)
{
	// restore old lights
	if (!oldIntensity.empty())
		r->SetLightIntensity(oldIntensity.size(), oldIntensity.data());
	r->SetAmbientColor(oldAmbient);
}

void ModelBody::RenderModel(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform, const bool setLighting)
{
	std::vector<float> oldIntensity;
	Color oldAmbient;
	if (setLighting)
		SetLighting(r, camera, oldIntensity, oldAmbient);

	matrix4x4d m2 = GetInterpOrient();
	m2.SetTranslate(GetInterpPosition());
	matrix4x4d t = viewTransform * m2;

	//double to float matrix
	matrix4x4f trans;
	for (int i = 0; i < 12; i++)
		trans[i] = float(t[i]);
	trans[12] = viewCoords.x;
	trans[13] = viewCoords.y;
	trans[14] = viewCoords.z;
	trans[15] = 1.0f;

	m_model->Render(trans);

	if (setLighting)
		ResetLighting(r, oldIntensity, oldAmbient);
}

void ModelBody::TimeStepUpdate(const float timestep)
{
	if (m_idleAnimation)
		// step animation by timestep/total length, loop to 0.0 if it goes >= 1.0
		m_idleAnimation->SetProgress(fmod(m_idleAnimation->GetProgress() + timestep / m_idleAnimation->GetDuration(), 1.0));

	m_model->UpdateAnimations();
}
