#include "RocketCameraElement.h"
#include "Pi.h"
#include "Frame.h"
#include "Body.h"
#include "StarSystem.h"
#include "render/Render.h"
#include "Space.h"

bool RocketCameraElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	dimensions.x = 256.0f;
	dimensions.y = 256.0f;
	return true;
}

void RocketCameraElement::UpdateFromStash(const RocketCamera &camera)
{
	m_camera = camera;
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

static void get_near_far_clip_plane(float *near, float *far)
{
	if (Render::AreShadersEnabled()) {
		/* If vertex shaders are enabled then we have a lovely logarithmic
		 * z-buffer stretching out from 0.1mm to 10000km! */
		*near = 0.0001f;
		*far = 10000000.0f;
	} else {
		/* Otherwise we have the usual hopelessly crap z-buffer */
		*near = 10.0f;
		*far = 1000000.0f;
	}
}

void RocketCameraElement::OnUpdate()
{
	if (!m_camera.body) return;

	// make temporary camera frame at the body
	m_camFrame = new Frame(m_camera.body->GetFrame(), "camera", Frame::TEMP_VIEWING);

	// interpolate between last physics tick position and current one,
	// to remove temporal aliasing
	matrix4x4d camOrient = m_camera.body->GetInterpolatedTransform();
	vector3d camPos = vector3d(camOrient[12], camOrient[13], camOrient[14]);
	camOrient.ClearToRotOnly();
	m_camFrame->SetPosition(camPos + (camOrient * m_camera.pos));

	camOrient.ClearToRotOnly();
	matrix4x4d camRot = matrix4x4d::Identity() * camOrient * m_camera.orient;
	m_camFrame->SetRotationOnly(camRot);

	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	m_camFrame->ClearMovement();
}

void RocketCameraElement::OnRender()
{
	if (!m_camera.body) return;

	float x = GetAbsoluteLeft();
	float y = GetAbsoluteTop();
	float width = GetClientWidth();
	float height = GetClientHeight();

	glColor3f(0,0,0);
	glBegin(GL_QUADS); {
		glVertex2f(x, y);
		glVertex2f(x, y+height);
		glVertex2f(x+width, y+height);
		glVertex2f(x+width, y);
	} glEnd();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float znear, zfar;
	get_near_far_clip_plane(&znear, &zfar);
	const float left = tan(DEG2RAD(20.0f)) * znear;	// XXX hardcoded fov to 40deg
	const float fracH = left * height/width;
	glFrustum(-left, left, -fracH, fracH, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(x, GetContext()->GetDimensions().y-(y+height), GLsizei(width), GLsizei(height));

	/* XXX not sure if background is useful/desirable
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
	*/

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
	{
		get_near_far_clip_plane(&znear, &zfar);
		Render::State::SetZnearZfar(znear, zfar);
	}

	Space::Render(m_camFrame);

	m_camera.body->GetFrame()->RemoveChild(m_camFrame);
	delete m_camFrame;

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_DEPTH_TEST);

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


class RocketCameraElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new RocketCameraElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void RocketCameraElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new RocketCameraElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("camera", instancer);
	instancer->RemoveReference();
}
