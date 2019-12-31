// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_IMAGE_H
#define UI_IMAGE_H

#include "SmartPtr.h"
#include "Widget.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "vector2.h"

namespace UI {

	class Image : public Widget {
	public:
		virtual Point PreferredSize();
		virtual void Draw();

		// SetHeightLines sets the widget's preferred size to fit the image to a
		// specified number of text lines in the widget's current font.
		Image *SetHeightLines(Uint32 lines);

		// SetNaturalSize sets the widget's preferred size to match the size of
		// the image data.
		Image *SetNaturalSize();

		// SetTransform applies a uniform scaling and an offset operation to the
		// image at display time (this can be used to zoom in on the image).
		// Parameter `centre' specifies what point in the image to centre in the
		// widget. This is specified in normalised image coordinates, so:
		//  0,0 -- centre the image
		//  1,1 -- centre on the bottom-right corner of the image
		//  -1,1 -- centre on the bottom-left corner of the image
		void SetTransform(float scale, const vector2f &centre);

		// SetPreserveAspect determines how the image content is scaled to fit the
		// Image widget. If true, the image is scaled uniformly and centered;
		// if false then the image is stretched to fill the whole Image widget.
		// Note that this is separate from the widget's size control flags, which
		// determines how the widget itself is sized within the available layout
		// area.
		void SetPreserveAspect(bool preserve_aspect);

	protected:
		friend class Context;
		Image(Context *context, const std::string &filename, Uint32 sizeControlFlags);

	private:
		RefCountedPtr<Graphics::Texture> m_texture;
		RefCountedPtr<Graphics::Material> m_material;
		std::unique_ptr<Graphics::Drawables::TexturedQuad> m_quad;
		Point m_initialSize;

		vector2f m_centre;
		float m_scale;
		bool m_preserveAspect;
		bool m_needsRefresh;
	};

} // namespace UI

#endif
