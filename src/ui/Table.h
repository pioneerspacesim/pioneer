// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_TABLE_H
#define UI_TABLE_H

#include "Container.h"
#include "Slider.h"
#include "Event.h"

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
	void ClearRows();

	Table *SetRowSpacing(int spacing);
	Table *SetColumnSpacing(int spacing);

	enum RowAlignDirection { // <enum scope='UI::Table' name=UITableRowAlignDirection prefix=ROW_ public>
		ROW_TOP,
		ROW_CENTER,
		ROW_BOTTOM
	};

	enum ColumnAlignDirection { // <enum scope='UI::Table' name=UITableColumnAlignDirection prefix=COLUMN_ public>
		COLUMN_LEFT,
		COLUMN_CENTER,
		COLUMN_RIGHT,
		COLUMN_JUSTIFY
	};

	Table *SetRowAlignment(RowAlignDirection dir);
	Table *SetColumnAlignment(ColumnAlignDirection dir);

	Table *SetHeadingFont(Font font);

	Table *SetMouseEnabled(bool enabled);

	void SetScrollPosition(float v);

	sigc::signal<void,unsigned int> onRowClicked;

protected:
	virtual void HandleInvisible();

private:

	class LayoutAccumulator {
	public:
		LayoutAccumulator() : m_columnSpacing(0), m_preferredWidth(0), m_columnAlignment(COLUMN_LEFT) {}

		void AddRow(const std::vector<Widget*> &widgets);
		void Clear();

		bool Empty() const { return m_columnWidth.empty(); }

		void SetColumnSpacing(int spacing) { m_columnSpacing = spacing; }
		void SetColumnAlignment(ColumnAlignDirection dir) { m_columnAlignment = dir; }

		void ComputeForWidth(int availWidth);

		int GetPreferredWidth() const { return m_preferredWidth; }

		const std::vector<int> &ColumnWidth() const { return m_columnWidth; }
		const std::vector<int> &ColumnLeft() const { return m_columnLeft; }

	private:
		std::vector<int> m_columnWidth;
		std::vector<int> m_columnLeft;
		int m_columnSpacing;
		int m_preferredWidth;
		ColumnAlignDirection m_columnAlignment;
	};


	class Inner: public Container {
	public:
		Inner(Context *context, LayoutAccumulator &layout);

		virtual Point PreferredSize();
		virtual void Layout();
		virtual void Draw();

		void AddRow(const std::vector<Widget*> &widgets);
		void Clear();

		void AccumulateLayout();

		void SetRowSpacing(int spacing);
		void SetColumnSpacing(int spacing);

		void SetRowAlignment(RowAlignDirection dir);

		void SetMouseEnabled(bool enabled) { m_mouseEnabled = enabled; }

		sigc::signal<void,unsigned int> onRowClicked;

	protected:
		virtual void HandleClick();

	private:
		int RowUnderPoint(const Point &pt, int *out_row_top = 0, int *out_row_bottom = 0) const;

		LayoutAccumulator &m_layout;
		std::vector< std::vector<Widget*> > m_rows;
		std::vector<int> m_rowHeight;
		Point m_preferredSize;
		int m_rowSpacing;
		RowAlignDirection m_rowAlignment;
		bool m_dirty;

		bool m_mouseEnabled;
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

