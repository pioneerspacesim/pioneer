// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIADJUSTMENT_H
#define _GUIADJUSTMENT_H

/* for scrollbars to fiddle with */
namespace Gui {
	class Adjustment {
		public:
			Adjustment(): m_value(0) {}
			float GetValue() { return m_value; }
			void SetValue(float v) {
				m_value = (v>0?(v<1?v:1):0);
				onValueChanged.emit();
			}
			sigc::signal<void> onValueChanged;
		private:
			float m_value;
	};
}

#endif /* _GUIADJUSTMENT_H */
