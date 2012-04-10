#ifndef _UI_LABEL_H
#define _UI_LABEL_H

#include "Widget.h"
#include "SmartPtr.h"

namespace UI {

class TextLayout;

class Label: public Widget {
public:
	virtual Metrics GetMetrics(const vector2f &hint);
	virtual void Draw();

protected:
	friend class Context;
	Label(Context *context, const std::string &text);

private:
	std::string m_text;
	ScopedPtr<TextLayout> m_layout;
};

}

#endif
