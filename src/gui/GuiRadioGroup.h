// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#ifndef _GUIRADIOGROUP_H
#define _GUIRADIOGROUP_H

#include "Gui.h"
#include <list>

namespace Gui {
	class RadioGroup {
	public:
		RadioGroup() {};
		virtual ~RadioGroup() {};
		void Add(ISelectable *b);
		void SetSelected(int member_idx);
	private:
		void OnSelected(ISelectable *b);
		std::list<ISelectable*> m_members;
	};
}

#endif /* _GUIRADIOGROUP_H */
