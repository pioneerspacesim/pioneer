#ifndef _GUITOOLTIP_H
#define _GUITOOLTIP_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class ToolTip: public Widget {
	public:
		ToolTip(const char *text);
		ToolTip(std::string &text);
		virtual void Draw();
		virtual ~ToolTip();
		virtual void GetSizeRequested(float size[2]);
		void SetText(const char *text);
		void SetText(std::string &text);
	private:
		void CalcSize();
		std::string m_text;
		TextLayout *m_layout;
		Uint32 m_createdTime;
	};
}

#endif /* _GUITOOLTIP_H */
