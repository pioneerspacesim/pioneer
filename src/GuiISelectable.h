#ifndef _GUIISELECTABLE_H
#define _GUIISELECTABLE_H

namespace Gui {
	class ISelectable {
	public:
		sigc::signal<void, ISelectable*> onSelect;
		virtual void SetSelected(bool) = 0;
	};
}

#endif /* _GUIISELECTABLE_H */
