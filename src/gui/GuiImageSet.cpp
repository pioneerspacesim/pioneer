#include "Gui.h"

namespace Gui {

ImageSet::ImageSet() : Widget()
{
	m_eventMask = EVENT_MOUSEDOWN;
	m_imagesVisible = true;
	m_imagesClickable = true;
}

bool ImageSet::OnMouseDown(Gui::MouseButtonEvent *e)
{
	if ((e->button == 1) && (m_imagesClickable)) {
		for (std::vector<ImageSetItem>::iterator i = m_items.begin(); i != m_items.end(); ++i) {
			if ((fabs(e->x - (*i).pos.x) < 10.0f) &&
			    (fabs(e->y - (*i).pos.y) < 10.0f)) {
				(*i).onClick();
				return false;
			}
		}
	}
	return true;
}

bool ImageSet::CanPutItem(float x, float y)
{
	for (std::vector<ImageSetItem>::iterator i = m_items.begin(); i != m_items.end(); ++i) {
		if ((fabs(x-(*i).pos.x) < 5.0f) &&
		    (fabs(y-(*i).pos.y) < 5.0f)) return false;
	}
	return true;
}

void ImageSet::Add(
	RefCountedPtr<Graphics::Texture> texture, sigc::slot<void> onClick,
	const vector2f &pos, const vector2f &size, const vector2f &texPos,
	const vector2f &texSize, const Color &tint
) {
	if (CanPutItem(pos.x, pos.y)) {
		m_items.push_back(ImageSetItem(texture, onClick, pos, size, texPos, texSize, tint));
	}
}

void ImageSet::Clear()
{
	m_items.clear();
}

void ImageSet::Draw()
{
	if (!m_imagesVisible) return;

	Screen::GetRenderer()->SetBlendMode(Graphics::BLEND_ALPHA);

	for (std::vector<ImageSetItem>::iterator i = m_items.begin(); i != m_items.end(); ++i) {
		Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
		
		vector2f pos = (*i).pos;
		vector2f size = (*i).size;
		vector2f texPos = (*i).texPos;
		vector2f texSize = (*i).texSize;

		va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(texPos.x,           texPos.y));
		va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
		va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(texPos.x+texSize.x, texPos.y));
		va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));

		// Create material on first use. Bit of a hack.
		if (!(*i).material.Valid()) {
			Graphics::MaterialDescriptor desc;
			desc.textures = 1;
			(*i).material.Reset(Screen::GetRenderer()->CreateMaterial(desc));
			(*i).material->texture0 = (*i).texture.Get();
		}
		(*i).material->diffuse = (*i).tint;
		Screen::GetRenderer()->DrawTriangles(&va, (*i).material.Get(), Graphics::TRIANGLE_STRIP);
	}

	Screen::GetRenderer()->SetBlendMode(Graphics::BLEND_SOLID);
}

void ImageSet::GetSizeRequested(float size[2])
{
	size[0] = 800.0f;
	size[1] = 600.0f;
}

}
