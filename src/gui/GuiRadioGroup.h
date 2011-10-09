#ifndef _GUIRADIOGROUP_H
#define _GUIRADIOGROUP_H

#include "Gui.h"
#include <list>

namespace Gui {
	class RadioGroup {
	public:
		RadioGroup() {};
		virtual ~RadioGroup();
		void Add(ISelectable *b);
		void SetSelected(int member_idx);
	private:
		void OnSelected(ISelectable *b);
		std::list<ISelectable*> m_members;
	};
}

#endif /* _GUIRADIOGROUP_H */
