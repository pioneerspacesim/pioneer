#ifndef _CAMERA_H
#define _CAMERA_H

#include "vector3.h"
#include "matrix4x4.h"
#include "Background.h"

class Body;
class Frame;

class Camera {
public:
	Camera(const Body *body, float width, float height);
	virtual ~Camera();

	float GetWidth() { return m_width; }
	float GetHeight() { return m_height; }

	void Update();
	void Draw();

	const Body *GetBody() const { return m_body; }

	void SetPosition(vector3d pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	void SetOrientation(matrix4x4d orient) { m_orient = orient; m_orient.ClearToRotOnly(); }
	matrix4x4d GetOrientation() const { return m_orient; }

	const Frame *GetFrame() const { return m_camFrame; }

	void SetFov(float ang);

private:
	void UpdateMatrices();

	const Body *m_body;

	float m_width;
	float m_height;

	vector3d m_pos;
	matrix4x4d m_orient;

	Background::Starfield m_starfield;
	Background::MilkyWay m_milkyWay;

	Frame *m_camFrame;

	bool m_shadersEnabled;

	double m_fov;

	GLfloat m_frustumLeft;
	GLfloat m_frustumTop;

	GLdouble m_modelMatrix[16];
	GLdouble m_projMatrix[16];
	GLint m_viewport[4];

	struct SortBody {
		double dist;
		vector3d viewCoords;
		matrix4x4d viewTransform;
		Body *b;
		Uint32 bodyFlags;
	};

	std::list<SortBody> m_sortedBodies;
};

#endif
