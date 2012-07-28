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
	Camera(const Body *body, float width, float height, float fovY, float nearClip, float farClip);
	virtual ~Camera();

	void Update();
	void Draw(Graphics::Renderer *r);

	const Body *GetBody() const { return m_body; }

	// camera position relative to the body
	void SetPosition(const vector3d &pos) { m_pose.SetTranslate(pos); }
	vector3d GetPosition() const { return m_pose.GetTranslate(); }

	// camera orientation relative to the body
	void SetOrientation(const matrix4x4d &orient) { m_pose.SetRotationOnly(orient); }
	matrix4x4d GetOrientation() const { matrix4x4d m(m_pose); m.ClearToRotOnly(); return m; }

	// only valid between Update() and Draw()
	const Frame *GetFrame() const { return m_camFrame; }

	// camera-specific light with attached body
	class Light : public Graphics::Light {
	public:
		Light(const Body *b, Graphics::Light::LightType t, const vector3f &pos, const Color &diffuse, const Color &ambient, const Color &specular) :
			Graphics::Light(t, pos, diffuse, ambient, specular), m_body(b)
		{}
		
		const Body *GetBody() const { return m_body; }
	
	private:
		const Body *m_body;
	};

	// lights with properties in camera space
	const std::vector<Light> &GetLights() const { return m_lights; }
	const int GetNumLights() const { return m_lights.size(); }

	// get the frustum. use for projection
	const Graphics::Frustum &GetFrustum() const { return m_frustum; }

protected:
	bool m_showCameraBody;

private:
	void OnBodyDeleted();
	sigc::connection m_onBodyDeletedConnection;

	void DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	const Body *m_body;
	float m_width;
	float m_height;
	float m_fovAng;
	float m_zNear;
	float m_zFar;

	Graphics::Frustum m_frustum;

	matrix4x4d m_pose;

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

	std::vector<Camera::Light> m_lights;

	Graphics::Renderer *m_renderer;
};

#endif
