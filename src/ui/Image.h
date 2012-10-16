// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_IMAGE_H
#define UI_IMAGE_H

#include "Widget.h"
#include "SmartPtr.h"
#include "gui/GuiTexturedQuad.h"

namespace UI {

class Image: public Widget {
public:
	enum StretchMode { // <enum scope='UI::Image' name=UIImageStretchMode prefix=STRETCH_>
		STRETCH_PRESERVE,   // preserve ratio
		STRETCH_MAX         // stretch to entire area allocated by container
	};

	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Image(Context *context, const std::string &filename, StretchMode stretchMode);

private:
	ScopedPtr<Gui::TexturedQuad> m_quad;
	StretchMode m_stretchMode;
	Point m_initialSize;
};

}

#endif
