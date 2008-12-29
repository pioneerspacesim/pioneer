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
		virtual bool OnMouseDown(MouseButtonEvent *e);
		void SelectPage(int page);
		sigc::signal<void,int> onSelectPage;
	private:
		bool IsLabelWidget(const Widget *);
		void ShuffleLabels();
		typedef std::list< std::pair<Widget*,Widget*> > pagecontainer_t;
		pagecontainer_t m_pages;
		int m_page;
	};
}

#endif /* _GUITABBED_H */
