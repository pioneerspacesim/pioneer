// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CAMERA_H
#define _CAMERA_H

#include "Color.h"
#include "FrameId.h"
#include "RefCounted.h"
#include "graphics/Frustum.h"
#include "graphics/Light.h"
#include "matrix4x4.h"
#include "vector3.h"

#include <list>
#include <memory>
#include <vector>

class Body;
class Frame;
class SpaceStation;

namespace Graphics {
	class Material;
	class Renderer;
} // namespace Graphics

class CameraContext : public RefCounted {
public:
	// camera for rendering to width x height with view frustum properties
	CameraContext(float width, float height, float fovAng, float zNear, float zFar);
	~CameraContext();

	float GetWidth() const { return m_width; }
	float GetHeight() const { return m_height; }
	float GetFovAng() const { return m_fovAng; }
	float GetZNear() const { return m_zNear; }
	float GetZFar() const { return m_zFar; }

	// Set the field of view of this camera context.
	// Should be called before ApplyDrawTransforms.
	void SetFovAng(float newAng);

	// frame to position the camera relative to
	void SetCameraFrame(FrameId frame) { m_frame = frame; }
	// return the parent frame of this camera
	FrameId GetCameraFrame() const { return m_frame; }

	// camera position relative to the frame origin
	void SetCameraPosition(const vector3d &pos) { m_pos = pos; }

	// camera orientation relative to the frame origin
	void SetCameraOrient(const matrix3x3d &orient) { m_orient = orient; }

	const vector3d GetCameraPos() const { return m_pos; }
	const matrix3x3d &GetCameraOrient() const { return m_orient; }

	// get the frustum. use for projection
	const Graphics::Frustum &GetFrustum() const { return m_frustum; }

	const matrix4x4f &GetProjectionMatrix() const { return m_projMatrix; }

	// generate and destroy the camere frame, used mostly to transform things to camera space
	void BeginFrame();
	void EndFrame();

	FrameId GetTempFrame() const { return m_camFrame; }

	// apply projection and modelview transforms to the renderer
	void ApplyDrawTransforms(Graphics::Renderer *r);

private:
	float m_width;
	float m_height;
	float m_fovAng;
	float m_zNear;
	float m_zFar;

	Graphics::Frustum m_frustum;
	matrix4x4f m_projMatrix;

	FrameId m_frame;
	vector3d m_pos;
	matrix3x3d m_orient;

	FrameId m_camFrame;
};

class Camera {
public:
	Camera(RefCountedPtr<CameraContext> context, Graphics::Renderer *renderer);

	const CameraContext *GetContext() const { return m_context.Get(); }

	void Update();
	void Draw(const Body *excludeBody = nullptr);

	// camera-specific light with attached source body
	class LightSource {
	public:
		LightSource(const Body *b, Graphics::Light &light) :
			m_body(b),
			m_light(light) {}

		const Body *GetBody() const { return m_body; }
		const Graphics::Light &GetLight() const { return m_light; }

	private:
		const Body *m_body;
		Graphics::Light m_light;
	};

	struct Shadow {
		vector3d centre;
		float srad;
		float lrad;

		bool operator<(const Shadow &other) const { return srad / lrad < other.srad / other.lrad; }
	};

	static void CalcLighting(const Body *b, const std::vector<Camera::LightSource> &lightSources, double &ambient, double &direct);
	static void CalcInteriorLighting(const Body* b, const std::vector<SpaceStation *> &stations, Color4ub &sLight, double &sFac);

	static void CalcShadows(const Body *lightBody, const Body *b, std::vector<Shadow> &shadowsOut);
	static float ShadowedIntensity(const Body *lightBody, const Body *b);

	void PrincipalShadows(const Body *b, const int n, std::vector<Shadow> &shadowsOut) const;

	// lights with properties in camera space
	const std::vector<LightSource> &GetLightSources() const { return m_lightSources; }
	int GetNumLightSources() const { return static_cast<Uint32>(m_lightSources.size()); }

	// Sets renderer lighting so that the body is renderer appropiately lit
	void PrepareLighting(const Body* b, bool doAtmosphere, bool doInteriors) const;
	// Restores default lights, use after rendering an object with custom lighting
	// to avoid those bleeding into other objects
	void RestoreLighting() const;

private:
	std::vector<float> m_oldLightIntensities;

	RefCountedPtr<CameraContext> m_context;
	Graphics::Renderer *m_renderer;

	std::unique_ptr<Graphics::Material> m_billboardMaterial;

	// temp attrs for sorting and drawing
	struct BodyAttrs {
		Body *body;

		// camera position and orientation relative to the body
		vector3d viewCoords;
		matrix4x4d viewTransform;

		// body distance from camera
		double camDist;

		// body flags. DRAW_LAST is the interesting one
		Uint32 bodyFlags;

		// if true, calculate atmosphere-attenuated light intensity for the body
		bool calcAtmosphereLighting;
		// if true, calculate interior light intensity for the body
		bool calcInteriorLighting;

		// if true, draw object as billboard of billboardSize at billboardPos
		bool billboard;
		vector3f billboardPos;
		float billboardSize;
		Color billboardColor;

		// for sorting. "should a be drawn before b?"
		// NOTE: Add below function (thus an indirection) in order
		// to decouple Camera from Body.h
		static bool sort_BodyAttrs(const BodyAttrs &a, const BodyAttrs &b);
		friend bool operator<(const BodyAttrs &a, const BodyAttrs &b)
		{
			return sort_BodyAttrs(a, b);
		};
	};

	std::list<BodyAttrs> m_sortedBodies;
	// For interior check
	std::vector<SpaceStation *> m_spaceStations;
	std::vector<LightSource> m_lightSources;
};

#endif
