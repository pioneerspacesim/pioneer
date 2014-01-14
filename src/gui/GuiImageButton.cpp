// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"
#include "GuiImageButton.h"

namespace Gui {

ImageButton::ImageButton(const char *img_normal): Button()
{
	LoadImages(img_normal, 0);
}

ImageButton::ImageButton(const char *img_normal, const char *img_pressed): Button()
{
	LoadImages(img_normal, img_pressed);
}

ImageButton::~ImageButton()
{
	delete m_imgNormal;
	if (m_imgPressed) delete m_imgPressed;
}

void ImageButton::LoadImages(const char *img_normal, const char *img_pressed)
{
	m_imgNormal = new Image(img_normal);
	float size[2];
	m_imgNormal->GetSizeRequested(size);
	SetSize(size[0], size[1]);

	if (img_pressed) m_imgPressed = new Image(img_pressed);
	else m_imgPressed = 0;
}

void ImageButton::GetSizeRequested(float size[2])
{
	m_imgNormal->GetSizeRequested(size);
}

void ImageButton::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);
	Gui::Image *img;
	if (m_imgPressed && IsPressed())
		img = m_imgPressed;
	else
		img = m_imgNormal;
	if (GetEnabled())
		img->SetModulateColor(Color::WHITE);
	else
		img->SetModulateColor(Color(128,128,128,255));
	img->SetSize(size[0], size[1]);
	img->Draw();
}

void ImageButton::SetRenderDimensions(const float wide, const float high)
{
	assert(m_imgNormal);
	m_imgNormal->SetRenderDimensions(wide, high);
	if(m_imgPressed)
		m_imgPressed->SetRenderDimensions(wide, high);
}

}
