#include "Camera.h"
#include "Frame.h"
#include "StarSystem.h"
#include "TerrainBody.h"
#include "render/Render.h"
#include "Space.h"
#include "Player.h"
#include "Pi.h"
#include "Sfx.h"
#include "Game.h"

Camera::Camera(const Body *body, float width, float height) :
	m_body(body),
	m_width(width),
	m_height(height),
	m_fovAng(Pi::config.Float("FOV")),
	m_shadersEnabled(Render::AreShadersEnabled()),
	m_frustum(m_width, m_height, m_fovAng),
	m_pose(matrix4x4d::Identity()),
	m_camFrame(0)
{
	for (int i=0; i<4; i++)
		m_systemLightBodies[i] = NULL;
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

void Camera::position_system_lights(Frame *camFrame, Frame *frame, int &lightNum)
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

		m_systemLightBodies[lightNum] = frame->m_astroBody;

		lightNum++;
	}

	for (std::list<Frame*>::iterator i = frame->m_children.begin(); i!=frame->m_children.end(); ++i) {
		position_system_lights(camFrame, *i, lightNum);
	}
}

void Camera::Update()
{
	if (!m_body) return;

	if (m_shadersEnabled != Render::AreShadersEnabled()) {
		m_frustum = Render::Frustum(m_width, m_height, m_fovAng);
		m_shadersEnabled = !m_shadersEnabled;
	}

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
		attrs.lightIntensities[0]=attrs.lightIntensities[1]=attrs.lightIntensities[2]=attrs.lightIntensities[3]=1.0;
		m_sortedBodies.push_back(attrs);
	}

	// depth sort
	m_sortedBodies.sort();
}

void Camera::Draw()
{
	if (!m_body) return;

	m_frustum.Enable();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix4x4d trans2bg;
	Frame::GetFrameTransform(Pi::game->GetSpace()->GetRootFrame(), m_camFrame, trans2bg);
	trans2bg.ClearToRotOnly();
	Pi::game->GetSpace()->GetBackground().Draw(trans2bg);

	int num_lights = 0;
	position_system_lights(m_camFrame, Pi::game->GetSpace()->GetRootFrame(), num_lights);

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

		m_systemLightBodies[0] = NULL;

		num_lights++;
	}

	Render::State::SetNumLights(num_lights);

	float znear, zfar;
	Render::GetNearFarClipPlane(znear, zfar);
	Render::State::SetZnearZfar(znear, zfar);

	for (std::list<BodyAttrs>::iterator i = m_sortedBodies.begin(); i != m_sortedBodies.end(); ++i) {
		BodyAttrs *attrs = &(*i);

		double rad = attrs->body->GetClipRadius();

		if (!m_frustum.TestPointInfinite((*i).viewCoords, rad))
			continue;

		// draw spikes for far objects
		double screenrad = 500 * rad / attrs->camDist;      // approximate pixel size
		if (!attrs->body->IsType(Object::STAR) && screenrad < 2) {
			if (!attrs->body->IsType(Object::PLANET)) continue;
			// absolute bullshit
			double spikerad = (7 + 1.5*log10(screenrad)) * rad / screenrad;
			DrawSpike(spikerad, attrs->viewCoords, attrs->viewTransform);
		}
		else
			RenderBody(attrs);
	}

	Sfx::RenderAll(Pi::game->GetSpace()->GetRootFrame(), m_camFrame);
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

void Camera::RenderBody(BodyAttrs *attrs) {
	Body *b = attrs->body;
	for (int i=0; i < Render::State::GetNumLights(); i++) {
		Body *lightBody = m_systemLightBodies[i];
		if (!lightBody)
			continue;

		// Set up data for eclipses. All bodies are assumed to be spheres.
		const double lightRadius = lightBody->GetSBody()->GetRadius();
		if ( b->IsType(Object::TERRAINBODY) || b->IsType(Object::MODELBODY)) {
			const vector3d bLightPos = lightBody->GetPositionRelTo(b);
			const vector3d lightDir = bLightPos.Normalized();

			double bRadius = 0;
			if ( b->IsType(Object::TERRAINBODY)) {
				TerrainBody* tb = static_cast<TerrainBody*>(b);
				tb->ClearEclipses();
				bRadius = b->GetSBody()->GetRadius();
				// Set up data for "self-eclipse", i.e. "night":
				if (tb->GetSBody()->type == SBody::TYPE_PLANET_ASTEROID)
					// Insufficiently spherical - ignore:
					tb->SetLightDiscRadius(i, -1);
				else
					// Store arctan of the apparant angular radius:
					tb->SetLightDiscRadius(i, lightRadius / bLightPos.Length());
			}

			// Look for eclipsing third bodies:
			for (Space::BodyIterator ib2 = Pi::game->GetSpace()->BodiesBegin(); ib2 != Pi::game->GetSpace()->BodiesEnd(); ++ib2) {
				Body *b2 = *ib2;
				if ( b2 != b && b2 != lightBody && b2->IsType(Object::TERRAINBODY) ) {
					const double b2Radius = b2->GetSBody()->GetRadius();

					vector3d b2pos = b2->GetPositionRelTo(b);
					const double perpDist = lightDir.Dot(b2pos);

					if ( perpDist <= 0 || perpDist > bLightPos.Length())
						// b2 isn't between b and lightBody; no eclipse
						continue;

					if ( b->IsType(Object::TERRAINBODY)) {
						TerrainBody* tb = static_cast<TerrainBody*>(b);
						// Project to the plane perpendicular to lightDir, taking the line between the shadowed sphere
						// (b) and the light source as zero. Our calculations assume that the light source is at
						// infinity. All lengths are normalised such that b has radius 1. srad is then the radius of the
						// occulting sphere (b2), and lrad is the apparent radius of the light disc when considered to
						// be at the distance of b2, and projectedCentre is the normalised projected position of the
						// centre of b2 relative to the centre of b. The upshot is that from a point on b, with
						// normalised projected position p, the picture is of a disc of radius lrad being occulted by a
						// disc of radius srad centred at projectedCentre-p. To determine the light intensity at p, we
						// then just need to estimate the proportion of the light disc being occulted.
						const double srad = b2Radius/bRadius;
						const double lrad = (lightRadius/bLightPos.Length())*perpDist/bRadius;
						if (srad / lrad < 0.01) {
							// any eclipse would have negligible effect - ignore
							continue;
						}
						const vector3d projectedCentre = ( b2pos - perpDist*lightDir ) / bRadius;
						if (projectedCentre.Length() < 1 + srad + lrad) {
							// some part of b is (partially) eclipsed
							if (srad > projectedCentre.Length() + 1 + lrad) {
								// whole planet is completely shadowed, so we can just turn the light off when
								// rendering the body:
								attrs->lightIntensities[i] = 0.0;
							}
							else {
								// non-uniform lighting - just tell the body what's happening:
								tb->AddEclipse(i, projectedCentre, srad, lrad);
							}
						}
					}
					else {
						// b is a model; we light these uniformly, so just need to calculate the intensity:
						const double srad = b2Radius;
						const double lrad = (lightRadius/bLightPos.Length())*perpDist;
						const double prdist = ( b2pos - perpDist*lightDir ).Length();
						if (prdist < srad + lrad) {
							float intensity = 1.0;
							if (prdist < abs(srad-lrad))
								if (srad > lrad)
									// umbra
									intensity = 0.0;
								else
									// penumbra
									intensity = 1.0 - (srad*srad) / (lrad*lrad);
							else
								// antumbra - just linearly interpolate (TODO:better?)
								intensity = 1.0 - ((srad+lrad-prdist)*(
											(srad > lrad) ? 1 : (srad*srad) / (lrad*lrad))) /
									(srad+lrad-abs(srad-lrad));
							attrs->lightIntensities[i] *= intensity;
						}
					}
				}
			}
		}
	}
	attrs->Render();
}

void Camera::BodyAttrs::Render() {

	// dim lights if requested
	static const int lightTypes[3] = {GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR};
	float lightData[4][3][4];
	float tempCol[4];
	for (int i=0; i < Render::State::GetNumLights(); i++) {
		if (lightIntensities[i] < 1.0) {
			for (int j=0; j<3; j++) {
				glGetLightfv(GL_LIGHT0+i, lightTypes[j], lightData[i][j]);
				for (int k=0; k<4; k++)
					tempCol[k] = lightIntensities[i]*lightData[i][j][k];
				glLightfv(GL_LIGHT0+i, lightTypes[j], tempCol);
			}
		}
	}

	// render
	body->Render(viewCoords, viewTransform);

	// restore dimmed lights
	for (int i=0; i < Render::State::GetNumLights(); i++)
		if (lightIntensities[i] < 1.0)
				for (int j=0; j<3; j++)
					glLightfv(GL_LIGHT0+i, lightTypes[j], lightData[i][j]);
}

void Camera::DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	glPushMatrix();

	float znear, zfar;
	Render::GetNearFarClipPlane(znear, zfar);
	double newdist = znear + 0.5f * (zfar - znear);
	double scale = newdist / viewCoords.Length();

	glTranslatef(float(scale*viewCoords.x), float(scale*viewCoords.y), float(scale*viewCoords.z));

	Render::State::UseProgram(0);
	// face the camera dammit
	vector3d zaxis = viewCoords.Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	glMultMatrixd(&rot[0]);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// XXX WRONG. need to pick light from appropriate turd.
	GLfloat col[4];
	glGetLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	glColor4f(col[0], col[1], col[2], 1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	glColor4f(col[0], col[1], col[2], 0);

	const float spikerad = float(scale*rad);

	// bezier with (0,0,0) control points
	{
		vector3f p0(0,spikerad,0), p1(spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			glVertex3fv(&p[0]);
		}
	}
	{
		vector3f p0(spikerad,0,0), p1(0,-spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			glVertex3fv(&p[0]);
		}
	}
	{
		vector3f p0(0,-spikerad,0), p1(-spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			glVertex3fv(&p[0]);
		}
	}
	{
		vector3f p0(-spikerad,0,0), p1(0,spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			glVertex3fv(&p[0]);
		}
	}
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

