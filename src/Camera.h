#ifndef _CAMERA_H
#define _CAMERA_H

#include "Frustum.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "Background.h"
#include "Body.h"

class Frame;

class Camera {
public:
	Camera(const Body *body, float width, float height);
	virtual ~Camera();

	void Update();
	void Draw();

	const Body *GetBody() const { return m_body; }

	void SetPosition(vector3d pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	void SetOrientation(matrix4x4d orient) { m_orient = orient; m_orient.ClearToRotOnly(); }
	matrix4x4d GetOrientation() const { return m_orient; }

	const Frame *GetFrame() const { return m_camFrame; }

private:
	Frustum m_frustum;

	const Body *m_body;

	float m_width;
	float m_height;

	vector3d m_pos;
	matrix4x4d m_orient;

	Background::Starfield m_starfield;
	Background::MilkyWay m_milkyWay;

	Frame *m_camFrame;

	struct SortBody {
		double dist;
		vector3d viewCoords;
		matrix4x4d viewTransform;
		Body *b;
		Uint32 bodyFlags;

		// for sorting. "should a be drawn before b?"
		friend bool operator<(const SortBody &a, const SortBody &b) {
			// both drawing last; distance order
			if (a.bodyFlags & Body::FLAG_DRAW_LAST && b.bodyFlags & Body::FLAG_DRAW_LAST)
				return a.dist < b.dist;

			// a drawing last; draw b first
			if (a.bodyFlags & Body::FLAG_DRAW_LAST)
				return false;

			// b drawing last; draw a first
			if (b.bodyFlags & Body::FLAG_DRAW_LAST)
				return true;

			// both in normal draw; distance order
			return a.dist < b.dist;
		}
	};

	std::list<SortBody> m_sortedBodies;
};

#endif
