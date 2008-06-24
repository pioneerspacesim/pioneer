#include "libs.h"
#include "Gui.h"
#include "GuiImageButton.h"
#include "Pi.h"

namespace Gui {

ImageButton::ImageButton(const char *img_normal): Button()
{
	LoadImages(img_normal, NULL);
}

ImageButton::ImageButton(const char *img_normal, const char *img_pressed): Button()
{
	LoadImages(img_normal, img_pressed);
}

void ImageButton::LoadImages(const char *img_normal, const char *img_pressed)
{
	m_imgNormal = new Image(img_normal);
	float size[2];
	m_imgNormal->GetSizeRequested(size);
	SetSize(size[0], size[1]);

	if (img_pressed) m_imgPressed = new Image(img_pressed);
	else m_imgPressed = NULL;
}

void ImageButton::GetSizeRequested(float size[2])
{
	m_imgNormal->GetSizeRequested(size);
}

void ImageButton::Draw()
{
	float size[2];
	GetSize(size);
	Gui::Image *img;
	if (m_imgPressed && IsPressed())
		img = m_imgPressed;
	else
		img = m_imgNormal;
	img->SetSize(size[0], size[1]);
	img->Draw();
}

}
