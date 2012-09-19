#ifndef GUIIMAGESET_H
#define GUIIMAGESET_H

#include "GuiWidget.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "RefCounted.h"
#include "SmartPtr.h"
#include <vector>

namespace Graphics {
	class Renderer;
	class Material;
}

/*
 * Collection of clickable labels. Used by the WorldView for clickable
 * bodies, and SystemView, SectorView etc.
 */
namespace Gui {
class ImageSet: public Widget {
public:
	class ImageSetItem {
	public:
		ImageSetItem(
			RefCountedPtr<Graphics::Texture> texture_,
			sigc::slot<void> onClick_,
			const vector2f &pos_, const vector2f &size_, const vector2f &texPos_,
			const vector2f &texSize_, const Color &tint_
		) :
			texture(texture_),
			onClick(onClick_), pos(pos_), size(size_),
			texPos(texPos_), texSize(texSize_), tint(tint_)
		{}
		RefCountedPtr<Graphics::Texture> texture;
		sigc::slot<void> onClick;
		RefCountedPtr<Graphics::Material> material;
		vector2f pos;
		vector2f size;
		vector2f texPos;
		vector2f texSize;
		Color tint;
	};

	ImageSet();
	bool OnMouseDown(MouseButtonEvent *e);
	virtual void Draw();
	virtual void GetSizeRequested(float size[2]);
	void Clear();
	void Add(
		RefCountedPtr<Graphics::Texture> texture, sigc::slot<void> onClick,
		const vector2f &pos, const vector2f &size, const vector2f &texPos,
		const vector2f &texSize, const Color &tint
	);
	void SetImagesClickable(bool v) { m_imagesClickable = v; }
	void SetImagesVisible(bool v) { m_imagesVisible = v; }
private:
	bool CanPutItem(float x, float y);

	std::vector<ImageSetItem> m_items;
	bool m_imagesVisible;
	bool m_imagesClickable;
};
}

#endif /* GUIIMAGESET_H */
