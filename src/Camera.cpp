#include "Camera.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Player.h"
#include "Pi.h"
#include "Sfx.h"
#include "Game.h"
#include "Light.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"

using namespace Graphics;

Camera::Camera(const Body *body, float width, float height, float fovY, float znear, float zfar) :
	m_showCameraBody(true),
	m_body(body),
	m_width(width),
	m_height(height),
	m_fovAng(fovY),
	m_zNear(znear),
	m_zFar(zfar),
	m_frustum(m_width, m_height, m_fovAng, znear, zfar),
	m_pose(matrix4x4d::Identity()),
	m_camFrame(0),
	m_renderer(0)
{
	m_onBodyDeletedConnection = m_body->onDelete.connect(sigc::mem_fun(this, &Camera::OnBodyDeleted));
}

Camera::~Camera()
{
	if (m_onBodyDeletedConnection.connected())
		m_onBodyDeletedConnection.disconnect();

	if (m_camFrame) {
		m_body->GetFrame()->RemoveChild(m_camFrame);
		delete m_camFrame;
	}
}

void Camera::OnBodyDeleted()
{
	m_onBodyDeletedConnection.disconnect();
	m_body = 0;
}

static void position_system_lights(Frame *camFrame, Frame *frame, std::vector<Light> &lights)
{
	if (lights.size() > 3) return;
	// not using frame->GetSBodyFor() because it snoops into parent frames,
	// causing duplicate finds for static and rotating frame
	SBody *body = frame->m_sbody;

	if (body && (body->GetSuperType() == SBody::SUPERTYPE_STAR)) {
		matrix4x4d m;
		Frame::GetFrameTransform(frame, camFrame, m);
		vector3d lpos = (m * vector3d(0,0,0));
		double dist = lpos.Length() / AU;
		lpos *= 1.0/dist; // normalize

		const float *col = StarSystem::starRealColors[body->type];

		Color lightCol(col[0], col[1], col[2], 0.f);
		Color ambCol(0.f);
		vector3f lightpos(lpos.x, lpos.y, lpos.z);
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, lightpos, lightCol, ambCol, lightCol));
	}

	for (std::list<Frame*>::iterator i = frame->m_children.begin(); i!=frame->m_children.end(); ++i) {
		position_system_lights(camFrame, *i, lights);
	}
}

void Camera::Update()
{
	if (!m_body) return;

	// make temporary camera frame at the body
	m_camFrame = new Frame(m_body->GetFrame(), "camera", Frame::TEMP_VIEWING);

	// interpolate between last physics tick position and current one,
	// to remove temporal aliasing
	matrix4x4d bodyPose = m_body->GetInterpolatedTransform();
	m_camFrame->SetTransform(bodyPose * m_pose);

	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	m_camFrame->ClearMovement();

	// evaluate each body and determine if/where/how to draw it
	m_sortedBodies.clear();
	for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i) {
		Body *b = *i;

		// prepare attrs for sorting and drawing
		BodyAttrs attrs;
		attrs.body = b;
		Frame::GetFrameRenderTransform(b->GetFrame(), m_camFrame, attrs.viewTransform);
		attrs.viewCoords = attrs.viewTransform * b->GetInterpolatedPosition();
		attrs.camDist = attrs.viewCoords.Length();
		attrs.bodyFlags = b->GetFlags();
		m_sortedBodies.push_back(attrs);
	}

	// depth sort
	m_sortedBodies.sort();
}

void Camera::Draw(Renderer *renderer)
{
	if (!m_body) return;
	if (!renderer) return;

	m_renderer = renderer;

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	m_renderer->SetPerspectiveProjection(m_fovAng, m_width/m_height, m_zNear, m_zFar);
	m_renderer->SetTransform(matrix4x4f::Identity());
	m_renderer->ClearScreen();

	matrix4x4d trans2bg;
	Frame::GetFrameRenderTransform(Pi::game->GetSpace()->GetRootFrame(), m_camFrame, trans2bg);
	trans2bg.ClearToRotOnly();
	Pi::game->GetSpace()->GetBackground().Draw(renderer, trans2bg);

	// Pick up to four suitable system light sources (stars)
	std::vector<Light> lights;
	lights.reserve(4);
	position_system_lights(m_camFrame, Pi::game->GetSpace()->GetRootFrame(), lights);

	if (lights.empty()) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		// fake one up and give a little ambient light so that we can see and
		// so that things that need lights don't explode
		Color col(1.f);
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, vector3f(0.f), col, col, col));
	}

	renderer->SetLights(lights.size(), &lights[0]);

	for (std::list<BodyAttrs>::iterator i = m_sortedBodies.begin(); i != m_sortedBodies.end(); ++i) {
		BodyAttrs *attrs = &(*i);

		if (attrs->body == GetBody() && !m_showCameraBody) continue;

		double rad = attrs->body->GetClipRadius();
		if (!m_frustum.TestPointInfinite((*i).viewCoords, rad))
			continue;

		// draw spikes for far objects
		double screenrad = 500 * rad / attrs->camDist;      // approximate pixel size
		if (attrs->body->IsType(Object::PLANET) && screenrad < 2) {
			// absolute bullshit
			double spikerad = (7 + 1.5*log10(screenrad)) * rad / screenrad;
			DrawSpike(spikerad, attrs->viewCoords, attrs->viewTransform);
		}
		else if (screenrad >= 2 || attrs->body->IsType(Object::STAR) ||
					(attrs->body->IsType(Object::PROJECTILE) && screenrad > 0.25))
			attrs->body->Render(renderer, attrs->viewCoords, attrs->viewTransform);
	}

	Sfx::RenderAll(renderer, Pi::game->GetSpace()->GetRootFrame(), m_camFrame);
	UnbindAllBuffers();

	m_body->GetFrame()->RemoveChild(m_camFrame);
	delete m_camFrame;
	m_camFrame = 0;

	glPopAttrib();
}

void Camera::DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	// draw twinkly star-thing on faraway objects
	// XXX this seems like a good case for drawing in 2D - use projected position, then the
	// "face the camera dammit" bits can be skipped
	if (!m_renderer) return;

	const double newdist = m_zNear + 0.5f * (m_zFar - m_zNear);
	const double scale = newdist / viewCoords.Length();

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(scale*viewCoords.x, scale*viewCoords.y, scale*viewCoords.z);

	// face the camera dammit
	vector3d zaxis = viewCoords.Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	trans = trans * rot;

	m_renderer->SetDepthTest(false);
	m_renderer->SetBlendMode(BLEND_ALPHA_ONE);

	// XXX WRONG. need to pick light from appropriate turd.
	GLfloat col[4];
	glGetLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	col[3] = 1.f;

	VertexArray va(ATTRIB_POSITION | ATTRIB_DIFFUSE);

	const Color center(col[0], col[1], col[2], col[2]);
	const Color edges(col[0], col[1], col[2], 0.f);

	//center
	va.Add(vector3f(0.f), center);

	const float spikerad = float(scale*rad);

	// bezier with (0,0,0) control points
	{
		const vector3f p0(0,spikerad,0), p1(spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(spikerad,0,0), p1(0,-spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(0,-spikerad,0), p1(-spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(-spikerad,0,0), p1(0,spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}

	Material mat;
	mat.unlit = true;
	mat.vertexColors = true;

	glPushMatrix();
	m_renderer->SetTransform(trans);
	m_renderer->DrawTriangles(&va, &mat, TRIANGLE_FAN);
	m_renderer->SetBlendMode(BLEND_SOLID);
	m_renderer->SetDepthTest(true);
	glPopMatrix();
}

