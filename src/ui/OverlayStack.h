// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_OVERLAYSTACK_H
#define UI_OVERLAYSTACK_H

#include "Container.h"

namespace UI {

	class OverlayStack : public Container {
	public:
		virtual Point PreferredSize();
		virtual void Layout();

		OverlayStack *AddLayer(Widget *widget);
		void Clear();

	protected:
		friend class Context;
		OverlayStack(Context *context) :
			Container(context) {}
	};

} // namespace UI

#endif
