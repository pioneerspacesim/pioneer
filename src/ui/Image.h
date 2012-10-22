// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_IMAGE_H
#define UI_IMAGE_H

#include "Widget.h"
#include "SmartPtr.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"

namespace UI {

class Image: public Widget {
public:
	enum StretchMode { // <enum scope='UI::Image' name=UIImageStretchMode prefix=STRETCH_>
		STRETCH_PRESERVE_ASPECT,   // preserve ratio
		STRETCH_MAX         // stretch to entire area allocated by container
	};

	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Image(Context *context, const std::string &filename, StretchMode stretchMode);

private:
	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_material;
	StretchMode m_stretchMode;
	Point m_initialSize;
};

}

#endif
