#ifndef _CAMERA_H
#define _CAMERA_H

#include "render/RenderFrustum.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "Background.h"
#include "Body.h"

class Frame;

class Camera {
public:
	// create camera relative to the given body, for rendering to area width x height
	Camera(const Body *body, float width, float height);
	virtual ~Camera();

	void Update();
	void Draw();

	const Body *GetBody() const { return m_body; }

	// camera position relative to the body
	void SetPosition(vector3d pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	// camera orientation relative to the body
	void SetOrientation(matrix4x4d orient) { m_orient = orient; m_orient.ClearToRotOnly(); }
	matrix4x4d GetOrientation() const { return m_orient; }

	// only valid between Update() and Draw()
	const Frame *GetFrame() const { return m_camFrame; }

	// get the frustum. use for projection
	const Render::Frustum &GetFrustum() const { return m_frustum; }

private:
	void DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	const Body *m_body;

	float m_width;
	float m_height;
	float m_fovAng;

	bool m_shadersEnabled;

	Render::Frustum m_frustum;

	vector3d m_pos;
	matrix4x4d m_orient;

	Background::Starfield m_starfield;
	Background::MilkyWay m_milkyWay;

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
};

#endif
