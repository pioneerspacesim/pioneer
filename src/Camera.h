// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CAMERA_H
#define _CAMERA_H

#include "graphics/Frustum.h"
#include "graphics/Light.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "Background.h"
#include "Body.h"


class Frame;
namespace Graphics { class Renderer; }

class Camera {
public:
	// create camera relative to the given body, for rendering to area width x height
	Camera(float width, float height, float fovY, float nearClip, float farClip);
	virtual ~Camera();

	void Update();
	void Draw(Graphics::Renderer *r, const Body *excludeBody = 0);

	// frame to position the camera relative to
	void SetFrame(Frame *frame) { m_frame = frame; }
	Frame *GetFrame() const { return m_frame; }

	// camera position relative to the frame origin
	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	// camera orientation relative to the frame origin
	void SetOrient(const matrix3x3d &orient) { m_orient = orient; }
	const matrix3x3d &GetOrient() const { return m_orient; }

	// valid between Update() and Draw()
	const Frame *GetCamFrame() const { return m_camFrame; }

	// camera-specific light with attached source body
	class LightSource {
	public:
		LightSource(const Body *b, Graphics::Light light) : m_body(b), m_light(light) {}

		const Body *GetBody() const { return m_body; }
		const Graphics::Light &GetLight() const { return m_light; }

	private:
		const Body *m_body;
		Graphics::Light m_light;
	};

	// lights with properties in camera space
	const std::vector<LightSource> &GetLightSources() const { return m_lightSources; }
	const int GetNumLightSources() const { return m_lightSources.size(); }

	// get the frustum. use for projection
	const Graphics::Frustum &GetFrustum() const { return m_frustum; }

private:
	void DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	float m_width;
	float m_height;
	float m_fovAng;
	float m_zNear;
	float m_zFar;

	Graphics::Frustum m_frustum;

	vector3d m_pos;
	matrix3x3d m_orient;

	Frame *m_frame;
	Frame *m_camFrame;

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

		// for sorting. "should a be drawn before b?"
		friend bool operator<(const BodyAttrs &a, const BodyAttrs &b) {
			// both drawing last; distance order
			if (a.bodyFlags & Body::FLAG_DRAW_LAST && b.bodyFlags & Body::FLAG_DRAW_LAST)
				return a.camDist > b.camDist;

			// a drawing last; draw b first
			if (a.bodyFlags & Body::FLAG_DRAW_LAST)
				return false;

			// b drawing last; draw a first
			if (b.bodyFlags & Body::FLAG_DRAW_LAST)
				return true;

			// both in normal draw; distance order
			return a.camDist > b.camDist;
		}
	};

	std::list<BodyAttrs> m_sortedBodies;
	std::vector<LightSource> m_lightSources;

	Graphics::Renderer *m_renderer;
};

#endif
