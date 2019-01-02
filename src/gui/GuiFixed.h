// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIFIXED_H
#define _GUIFIXED_H
/*
 * Fixed position widget container.
 */

#include "GuiContainer.h"
#include "GuiWidget.h"

namespace Gui {
	class Fixed : public Container {
	public:
		Fixed(float w, float h);
		Fixed();
		void Add(Widget *child, float x, float y);
		void Remove(Widget *child);
		virtual ~Fixed();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		virtual void UpdateAllChildSizes();
		void SetSizeRequest(float x, float y);
		void SetSizeRequest(float size[2]);

	private:
		void _Init();
		float m_userWantedSize[2];
	};
} // namespace Gui

#endif /* _GUIFIXED_H */
