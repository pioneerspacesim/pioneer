// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_FACE_H
#define GAMEUI_FACE_H

#include "ui/Context.h"
#include "SmartPtr.h"
#include "graphics/Texture.h"

namespace GameUI {

class Face : public UI::Single {
public:
	Face(UI::Context *context, Uint32 flags = 0, Uint32 seed = 0);

	virtual UI::Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	Face *SetHeightLines(Uint32 lines);

	enum Flags { // <enum scope='GameUI::Face' name=GameUIFaceFlags public>
		RAND        = 0,
		MALE        = (1<<0),
		FEMALE      = (1<<1),
		GENDER_MASK   = 0x03,   // <enum skip>

		ARMOUR = (1<<2),
	};

private:
	UI::Point m_preferredSize;

	Uint32 m_flags;
	Uint32 m_seed;

	static RefCountedPtr<Graphics::Material> s_material;

	std::unique_ptr<Graphics::Texture> m_texture;
};

}

#endif
