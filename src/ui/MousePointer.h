// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_MOUSEPOINTER_H
#define UI_MOUSEPOINTER_H

#include "Image.h"

namespace UI {

class MousePointer: public Image {

protected:
    friend class Context;
    MousePointer(Context *context, const std::string &filename, const Point &hotspot) :
        Image(context, filename, UI::Widget::PRESERVE_ASPECT),
        m_hotspot(hotspot)
    {}

	const Point &GetHotspot() const { return m_hotspot; }

private:
    Point m_hotspot;
};

}

#endif
