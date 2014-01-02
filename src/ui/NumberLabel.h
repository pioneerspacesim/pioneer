// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_NUMBERLABEL_H
#define UI_NUMBERLABEL_H

#include "Label.h"
#include "PropertyMap.h"

namespace UI {

class NumberLabel : public Label {
public:
	enum Format { // <enum scope='UI::NumberLabel' name=UINumberLabelFormat prefix=FORMAT_>
		FORMAT_NUMBER,
		FORMAT_NUMBER_2DP,
		FORMAT_INTEGER,
		FORMAT_PERCENT,
		FORMAT_PERCENT_INTEGER,
		FORMAT_MONEY,
		FORMAT_MASS_TONNES
	};

	NumberLabel *SetValue(double v);
	double GetValue() const { return m_value; }

protected:
	friend class Context;
	NumberLabel(Context *context, Format format);

private:
	void BindValue(PropertyMap &p, const std::string &k);
	void BindValuePercent(PropertyMap &p, const std::string &k);

	Format m_format;
	double m_value;
};

}

#endif
