#include "Camera.h"
#include "Frame.h"
#include "StarSystem.h"
#include "render/Render.h"
#include "Space.h"
#include "Player.h"
#include "Pi.h"
#include "Sfx.h"

// min/max FOV in degrees
static const float FOV_MAX = 170.0f;
static const float FOV_MIN = 20.0f;

Camera::Camera(const Body *body, float width, float height) :
	m_frustum(width, height),
	m_body(body),
	m_pos(0.0),
	m_orient(matrix4x4d::Identity()),
	m_camFrame(0)
{
}

Camera::~Camera()
{
	if (m_camFrame) {
		m_body->GetFrame()->RemoveChild(m_camFrame);
		delete m_camFrame;
	}
}

static void position_system_lights(Frame *camFrame, Frame *frame, int &lightNum)
{
	if (lightNum > 3) return;
	// not using frame->GetSBodyFor() because it snoops into parent frames,
	// causing duplicate finds for static and rotating frame
	SBody *body = frame->m_sbody;

	if (body && (body->GetSuperType() == SBody::SUPERTYPE_STAR)) {
		int light;
		switch (lightNum) {
			case 3: light = GL_LIGHT3; break;
			case 2: light = GL_LIGHT2; break;
			case 1: light = GL_LIGHT1; break;
			default: light = GL_LIGHT0; break;
		}
		// position light at sol
		matrix4x4d m;
		Frame::GetFrameTransform(frame, camFrame, m);
		vector3d lpos = (m * vector3d(0,0,0));
		double dist = lpos.Length() / AU;
		lpos *= 1.0/dist; // normalize
		float lightPos[4];
		lightPos[0] = float(lpos.x);
		lightPos[1] = float(lpos.y);
		lightPos[2] = float(lpos.z);
		lightPos[3] = 0;

		const float *col = StarSystem::starRealColors[body->type];
		float lightCol[4] = { col[0], col[1], col[2], 0 };
		float ambCol[4] = { 0,0,0,0 };
		if (Render::IsHDREnabled()) {
			for (int i=0; i<4; i++) {
				// not too high or we overflow our float16 colorbuffer
				lightCol[i] *= float(std::min(10.0*StarSystem::starLuminosities[body->type] / dist, 10000.0));
			}
		}

		glLightfv(light, GL_POSITION, lightPos);
		glLightfv(light, GL_DIFFUSE, lightCol);
		glLightfv(light, GL_AMBIENT, ambCol);
		glLightfv(light, GL_SPECULAR, lightCol);
		glEnable(light);

		lightNum++;
	}

	for (std::list<Frame*>::iterator i = frame->m_children.begin(); i!=frame->m_children.end(); ++i) {
		position_system_lights(camFrame, *i, lightNum);
	}
}

void Camera::Update()
{
	m_frustum.Update();

	// make temporary camera frame at the body
	m_camFrame = new Frame(m_body->GetFrame(), "camera", Frame::TEMP_VIEWING);

	// interpolate between last physics tick position and current one,
	// to remove temporal aliasing
	matrix4x4d camOrient = m_body->GetInterpolatedTransform();
	vector3d camPos = vector3d(camOrient[12], camOrient[13], camOrient[14]);
	camOrient.ClearToRotOnly();
	m_camFrame->SetPosition(camPos + (camOrient * m_pos));

	camOrient.ClearToRotOnly();
	matrix4x4d camRot = matrix4x4d::Identity() * camOrient * m_orient;
	m_camFrame->SetRotationOnly(camRot);

	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	m_camFrame->ClearMovement();


	// evaluate each body and determine if/where/how to draw it
	const float *guiscale = Gui::Screen::GetCoords2Pixels();

	m_sortedBodies.clear();
	for (std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		Body *b = *i;

		// put the body in the sort list
		SortBody bz;
		vector3d pos = (*i)->GetInterpolatedPosition();
		Frame::GetFrameRenderTransform(b->GetFrame(), m_camFrame, bz.viewTransform);
		vector3d toBody = bz.viewTransform * pos;
		bz.viewCoords = toBody;
		bz.dist = toBody.Length();
		bz.bodyFlags = b->GetFlags();
		bz.b = b;
		m_sortedBodies.push_back(bz);

		// calculate and store projected position for labels etc
		// XXX sucks to do this for every camera
		// XXX rough copy of Gui::Screen::Project but avoiding the overhead of EnterOrtho/LeaveOrtho
		b->SetOnscreen(false);
		pos = b->GetInterpolatedPositionRelTo(m_camFrame);
		if (pos.z < 0 && m_frustum.ProjectPoint(pos, pos)) {
			pos.x = pos.x * guiscale[0];
			pos.y = Gui::Screen::GetHeight() - pos.y * guiscale[1];
			b->SetProjectedPos(pos);
			b->SetOnscreen(true);
		}
	}

	// depth sort
	m_sortedBodies.sort();
}

void Camera::Draw()
{
	m_frustum.Enable();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix4x4d trans2bg;
	Frame::GetFrameTransform(Space::rootFrame, m_camFrame, trans2bg);
	trans2bg.ClearToRotOnly();
	glPushMatrix();
	glMultMatrixd(&trans2bg[0]);
	glPushMatrix();
	glRotatef(40.0, 1.0,2.0,3.0);
	m_milkyWay.Draw();
	glPopMatrix();
	m_starfield.Draw();
	glPopMatrix();

	int num_lights = 0;
	position_system_lights(m_camFrame, Space::rootFrame, num_lights);

	if (num_lights == 0) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		// fake one up and give a little ambient light so that we can see and
		// so that things that need lights don't explode
		float lightPos[4] = { 0,0,0,0 };
		float lightCol[4] = { 1.0, 1.0, 1.0, 0 };
		float ambCol[4] = { 1.0,1.0,1.0,0 };

		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambCol);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
		glEnable(GL_LIGHT0);

		num_lights++;
	}

	Render::State::SetNumLights(num_lights);

	float znear, zfar;
	Render::GetNearFarClipPlane(znear, zfar);
	Render::State::SetZnearZfar(znear, zfar);

	for (std::list<SortBody>::iterator i = m_sortedBodies.begin(); i != m_sortedBodies.end(); i++) {
		double rad = (*i).b->GetBoundingRadius();

		// frustum cull. always draw stars because their glow extends past
		// their bounding radius
		// XXX remove this exception by adding a clip radius to stars that
		// includes their glow, otherwise the render can get expensive (stars
		// have terrain now)
		if (!(*i).b->IsType(Object::STAR) && !m_frustum.ContainsPoint((*i).viewCoords, rad))
			continue;

		double screenrad = 500 * rad / (*i).dist;      // approximate pixel size
		if (!(*i).b->IsType(Object::STAR) && screenrad < 2) {
			if (!(*i).b->IsType(Object::PLANET)) continue;
			// absolute bullshit
			double spikerad = (7 + 1.5*log10(screenrad)) * rad / screenrad;
			//DrawSpike(spikerad, (*i).viewCoords, (*i).viewTransform);
		}
		else
			(*i).b->Render((*i).viewCoords, (*i).viewTransform);
	}

	Sfx::RenderAll(Space::rootFrame, m_camFrame);
	Render::State::UseProgram(0);
	Render::UnbindAllBuffers();

	m_body->GetFrame()->RemoveChild(m_camFrame);
	delete m_camFrame;
	m_camFrame = 0;

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);

	glPopAttrib();

	m_frustum.Disable();
}
