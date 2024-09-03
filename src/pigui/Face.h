// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PIGUI_FACE_H
#define PIGUI_FACE_H

#include "FaceParts.h"
#include "Pi.h"
#include "SmartPtr.h"
#include "graphics/Drawables.h"
#include "graphics/Texture.h"

namespace PiGui {

	class Face : public RefCounted {
	public:
		Face(FaceParts::FaceDescriptor &face, Uint32 seed = 0);

		void *GetImTextureID();
		vector2f GetTextureSize();

		enum Flags { // <enum scope='PiGui::Face' name=PiGuiFaceFlags public>
			RAND = 0,
			MALE = (1 << 0),
			FEMALE = (1 << 1),
			GENDER_MASK = 0x03, // <enum skip>

			ARMOUR = (1 << 2),
		};

	private:
		Uint32 m_seed;

		RefCountedPtr<Graphics::Texture> m_texture;
	};

} // namespace PiGui

#endif
