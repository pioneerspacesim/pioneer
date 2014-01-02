// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUITABBED_H
#define _GUITABBED_H

#include "GuiContainer.h"

namespace Gui {
	class Tabbed: public Container
	{
	public:
		Tabbed();
		void AddPage(Widget *label, Widget *child);
		void Remove(Widget *child);
		virtual void Show();
		virtual void Hide();
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		virtual void UpdateAllChildSizes();
		virtual bool OnMouseDown(MouseButtonEvent *e);
		void SelectPage(int page);
		int GetCurrentPage() const { return m_page; }
		sigc::signal<void,int> onSelectPage;
		virtual void OnActivate();
	private:
		bool IsLabelWidget(const Widget *);
		void ShuffleLabels();
		typedef std::list< std::pair<Widget*,Widget*> > pagecontainer_t;
		pagecontainer_t m_pages;
		unsigned int m_page;
	};
}

#endif /* _GUITABBED_H */
