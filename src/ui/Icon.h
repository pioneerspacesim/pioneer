// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_ICON_H
#define UI_ICON_H

#include "IniConfig.h"
#include "SmartPtr.h"
#include "Widget.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "vector2.h"

namespace UI {

	class Icon : public Widget {
	public:
		virtual Point PreferredSize();
		virtual void Draw();

		Icon *SetColor(const Color &c)
		{
			m_color = c;
			return this;
		}

	protected:
		friend class Context;
		Icon(Context *context, const std::string &iconName);

	private:
		static IniConfig s_config;

		static RefCountedPtr<Graphics::Texture> s_texture;
		static vector2f s_texScale;

		static RefCountedPtr<Graphics::Material> s_material;

		Point m_texPos;
		Color m_color;
		std::unique_ptr<Graphics::Drawables::TexturedQuad> m_quad;
	};

} // namespace UI

#endif
