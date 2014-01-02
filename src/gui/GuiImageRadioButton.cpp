// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	PROFILE_SCOPED()
	float sz[2];
	GetSize(sz);
	if (m_pressed) {
		m_imgPressed->SetSize(sz[0], sz[1]);
		m_imgPressed->Draw();
	} else {
		m_imgNormal->SetSize(sz[0], sz[1]);
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
