// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIISELECTABLE_H
#define _GUIISELECTABLE_H

namespace Gui {
	class ISelectable {
	public:
		sigc::signal<void> onSelect;
		virtual void SetSelected(bool) = 0;
	};
}

#endif /* _GUIISELECTABLE_H */
