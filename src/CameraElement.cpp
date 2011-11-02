#include "CameraElement.h"
#include "Camera.h"

CameraElement::~CameraElement()
{
	if (m_camera)
		delete m_camera;
}

bool CameraElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	dimensions.x = 256.0f;
	dimensions.y = 256.0f;
	return true;
}

void CameraElement::UpdateFromStash(const CameraElementData &cameraData)
{
	if (m_camera) {
		if (m_camera->GetBody() == cameraData.body) {
			m_camera->SetPosition(cameraData.pos);
			m_camera->SetOrientation(cameraData.orient);
			return;
		}

		delete m_camera;
	}

	float width = GetClientWidth();
	float height = GetClientHeight();

	m_camera = new Camera(cameraData.body, width, height);
	m_camera->SetPosition(cameraData.pos);
	m_camera->SetOrientation(cameraData.orient);
}

void CameraElement::OnUpdate()
{
	if (!m_camera) return;
	m_camera->Update();
}

void CameraElement::OnRender()
{
	if (!m_camera) return;

	float x = GetAbsoluteLeft();
	float y = GetAbsoluteTop();
	float width = GetClientWidth();
	float height = GetClientHeight();

	float invy = GetContext()->GetDimensions().y-(y+height);

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(GLint(x), GLint(invy), GLsizei(width), GLsizei(height));
	glScissor(GLint(x), GLint(invy), GLsizei(width), GLsizei(height));
	glEnable(GL_SCISSOR_TEST);
	m_camera->Draw();
	glDisable(GL_SCISSOR_TEST);
	glPopAttrib();
}
