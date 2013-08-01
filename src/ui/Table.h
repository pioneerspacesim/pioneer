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

	Table *SetHeadingFont(Font font);

private:

	class LayoutAccumulator {
	public:
		LayoutAccumulator() : m_columnSpacing(0) {}

		void AddRow(const std::vector<Widget*> &widgets);
		void Clear();

		void SetColumnSpacing(int spacing);

		const std::vector<int> &ColumnWidth() const { return m_columnWidth; }
		const std::vector<int> &ColumnLeft() const { return m_columnLeft; }

	private:
		std::vector<int> m_columnWidth;

		void UpdateColumns();
		std::vector<int> m_columnLeft;
		int m_columnSpacing;
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

	private:
		LayoutAccumulator &m_layout;
		std::vector< std::vector<Widget*> > m_rows;
		std::vector<int> m_rowHeight;
		Point m_preferredSize;
		int m_rowSpacing;
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

