// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
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
	Point GetImageSize() const;

	virtual Point PreferredSize();
	virtual void Draw();

	// SetHeightLines sets the widget's preferred size to fit the image to a
	// specified number of text lines in the widget's current font.
	Image *SetHeightLines(Uint32 lines);

	// SetNaturalSize sets the widget's preferred size to match the size of
	// the image data.
	Image *SetNaturalSize();

protected:
	friend class Context;
	Image(Context *context, const std::string &filename, Uint32 sizeControlFlags);

private:
	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_material;
	Point m_initialSize;
};

}

#endif
