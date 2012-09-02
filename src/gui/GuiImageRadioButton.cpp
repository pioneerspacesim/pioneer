#include "libs.h"
#include "Gui.h"
#include "GuiImageRadioButton.h"

namespace Gui {

ImageRadioButton::ImageRadioButton(RadioGroup *g, const char *img_normal, const char *img_pressed): RadioButton(g)
{
	m_imgNormal = new Image(img_normal);
	m_imgPressed = new Image(img_pressed);
	float size[2];
	m_imgNormal->GetSizeRequested(size);
	SetSize(size[0], size[1]);
}

ImageRadioButton::~ImageRadioButton()
{
	delete m_imgNormal;
	delete m_imgPressed;
}

void ImageRadioButton::GetSizeRequested(float size[2])
{
	m_imgNormal->GetSizeRequested(size);
}

void ImageRadioButton::Draw()
{
	if (m_pressed) {
		m_imgPressed->Draw();
	} else {
		m_imgNormal->Draw();
	}
}

void ImageRadioButton::SetRenderDimensions(const float wide, const float high)
{
	assert(m_imgPressed && m_imgNormal);
	m_imgPressed->SetRenderDimensions(wide, high);
	m_imgNormal->SetRenderDimensions(wide, high);
}

}
