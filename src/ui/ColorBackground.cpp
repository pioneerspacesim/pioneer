// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ColorBackground.h"
#include "Context.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

namespace UI {

	ColorBackground::ColorBackground(Context *context, const Color &color) :
		Single(context),
		m_color(color)
	{
	}

	void ColorBackground::Draw()
	{
		GetContext()->GetSkin().DrawRectColor(m_color, Point(), GetSize());
		Container::Draw();
	}

} // namespace UI
