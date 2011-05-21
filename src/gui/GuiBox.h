#ifndef _GUIBOX_H
#define _GUIBOX_H
/*
 * Box oriented packing widget container.
 */

#include "GuiWidget.h"
#include "GuiContainer.h"

namespace Gui {
	enum BoxOrientation {
		BOX_HORIZONTAL = 0,
		BOX_VERTICAL = 1
	};

	class Box: public Container {
	public:
		Box(BoxOrientation orient);
		void PackStart(Widget *child, bool expand=false);
		void PackEnd(Widget *child, bool expand=false);
		void Remove(Widget *child);
		virtual ~Box();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		virtual void UpdateAllChildSizes();
		void SetSizeRequest(float size[2]);
		void SetSizeRequest(float x, float y);
		void SetSpacing(float spacing) { m_spacing = spacing; }
	private:
		void _Init();
		float m_wantedSize[2];
		float m_spacing;
		enum BoxOrientation m_orient;
	};

	class VBox: public Box {
	public:
		VBox(): Box(BOX_VERTICAL) {}
	};
	
	class HBox: public Box {
	public:
		HBox(): Box(BOX_HORIZONTAL) {}
	};
}

#endif /* _GUIBOX_H */

