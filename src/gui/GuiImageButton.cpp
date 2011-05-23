#include "libs.h"
#include "Gui.h"
#include "GuiImageButton.h"

namespace Gui {

ImageButton::ImageButton(const char *img_normal): Button()
{
	LoadImages(img_normal, NULL);
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
	if (GetEnabled())
		img->SetModulateColor(1,1,1,1);
	else
		img->SetModulateColor(.5,.5,.5,1);
	img->SetSize(size[0], size[1]);
	img->Draw();
}

}
