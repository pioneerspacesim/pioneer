// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "Color.h"
#include "CollMesh.h"
#include "FrameId.h"

class Shields;
class Geom;
class Camera;

namespace Graphics {
	class Renderer;
	class Light;
} // namespace Graphics

namespace SceneGraph {
	class Model;
	class Animation;
} // namespace SceneGraph

class ModelBody : public Body {
public:
	OBJDEF(ModelBody, Body, MODELBODY);
	ModelBody();
	ModelBody(const Json &jsonObj, Space *space);
	virtual ~ModelBody();
	void SetPosition(const vector3d &p) override;
	void SetOrient(const matrix3x3d &r) override;
	virtual void SetFrame(FrameId fId) override;
	// Colliding: geoms are checked against collision space
	void SetColliding(bool colliding);
	bool IsColliding() const { return m_colliding; }
	// Static: geoms are static relative to frame
	void SetStatic(bool isStatic);
	bool IsStatic() const { return m_isStatic; }
	const Aabb &GetAabb() const { return m_collMesh->GetAabb(); }
	SceneGraph::Model *GetModel() const { return m_model; }
	CollMesh *GetCollMesh() { return m_collMesh.Get(); }
	Geom *GetGeom() const { return m_geom; }

	void SetModel(const char *modelName);

	void RenderModel(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	virtual void TimeStepUpdate(const float timeStep) override;

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;

	Shields *GetShields() const { return m_shields.get(); }

private:
	void RebuildCollisionMesh();
	void DeleteGeoms();
	void AddGeomsToFrame(Frame *);
	void RemoveGeomsFromFrame(Frame *);
	void MoveGeoms(const matrix4x4d &, const vector3d &);

	bool m_isStatic;
	bool m_colliding;
	RefCountedPtr<CollMesh> m_collMesh;
	Geom *m_geom; //static geom
	std::string m_modelName;
	SceneGraph::Model *m_model;
	std::vector<Geom *> m_dynGeoms;
	SceneGraph::Animation *m_idleAnimation;
	std::unique_ptr<Shields> m_shields;
};

#endif /* _MODELBODY_H */
