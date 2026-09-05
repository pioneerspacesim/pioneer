// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TERRAINBODY_H
#define _TERRAINBODY_H

#include "Body.h"
#include "JsonFwd.h"
#include "matrix4x4.h"

class BaseSphere;
class Camera;
class Frame;
class Space;
class SystemBody;

namespace Graphics {
	class Renderer;
}

class TerrainBody : public Body {
public:
	OBJDEF(TerrainBody, Body, TERRAINBODY);

	void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	virtual void SubRender(Graphics::Renderer *r, const matrix4x4d &modelView, const vector3d &camPos) {}
	void SetFrame(FrameId fId) override;
	virtual bool OnCollision(Body *b, Uint32 flags, double relVel) override { return true; }
	double GetMass() const override { return m_mass; }
	double GetTerrainHeight(const vector3d &pos) const;
	// pos is a direction from the planet centre (need not be unit length).
	vector3d GetTerrainSurfacePoint(const vector3d &pos) const;
	// sampleSizeMeters is the approximate diameter of the area to sample (e.g. ship footprint).
	vector3d GetTerrainSurfaceNormal(const vector3d &pos, double sampleSizeMeters) const;
	const SystemBody *GetSystemBody() const override { return m_sbody; }

	// returns value in metres
	double GetMaxFeatureRadius() const { return m_maxFeatureHeight; }

	// implements calls to all relevant terrain management sub-systems
	static void OnChangeDetailLevel(Graphics::Renderer *r);

	const char *GetHeightFractalName() const;
	const char *GetColorFractalName() const;

protected:
	TerrainBody() = delete;
	TerrainBody(SystemBody *);
	TerrainBody(const Json &jsonObj, Space *space);
	virtual ~TerrainBody();

	void InitTerrainBody();

	void SaveToJson(Json &jsonObj, Space *space) override;

private:
	const SystemBody *m_sbody;
	double m_mass;
	std::unique_ptr<BaseSphere> m_baseSphere;
	double m_maxFeatureHeight;
};

#endif
