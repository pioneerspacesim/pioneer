// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_TABLE_H
#define UI_TABLE_H

#include "Container.h"
#include "Slider.h"

namespace UI {

class Table: public Container {
protected:
	friend class Context;
	Table(Context *context);

public:
	virtual Point PreferredSize();
	virtual void Layout();

	Table *SetHeadingRow(const WidgetSet &set);
	Table *AddRow(const WidgetSet &set);

	Table *SetRowSpacing(int spacing);
	Table *SetColumnSpacing(int spacing);
	Table *SetSpacing(int spacing);

	Table *SetHeadingFont(Font font);

private:

	class LayoutAccumulator {
	public:
		void AddRow(const std::vector<Widget*> &widgets);
		void Clear();

		void SetSpacing(int h, int v);

		const std::vector<int> &ColumnWidths() const { return m_columnWidths; }
		const Point &GetSize() const { return m_size; }
	private:
		std::vector<int> m_columnWidths;
		Point m_size;
	};


	class Inner: public Container {
	public:
		Inner(Context *context, LayoutAccumulator &layout);

		virtual Point PreferredSize();
		virtual void Layout();

		void AddRow(const std::vector<Widget*> &widgets);
		void Clear();

		void AccumulateLayout();

		void SetRowSpacing(int spacing);
		void SetColumnSpacing(int spacing);
		void SetSpacing(int spacing);

	private:
		LayoutAccumulator &m_layout;
		std::vector< std::vector<Widget*> > m_rows;
		std::vector< std::vector<Point> > m_preferredSizes;
		Point m_preferredSize;
		int m_rowSpacing;
		int m_columnSpacing;
		bool m_dirty;
	};

	LayoutAccumulator m_layout;
	bool m_dirty;

	RefCountedPtr<Inner> m_header;
	RefCountedPtr<Inner> m_body;

	RefCountedPtr<Slider> m_slider;

	sigc::connection m_onMouseWheelConn;

	void OnScroll(float value);
	bool OnMouseWheel(const MouseWheelEvent &event);
};

}

#endif

