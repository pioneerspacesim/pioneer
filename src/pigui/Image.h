// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PIGUI_IMAGE_H
#define PIGUI_IMAGE_H

#include "Pi.h"
#include "SmartPtr.h"
#include "graphics/TextureBuilder.h"

namespace PiGui {

	class Image : public RefCounted {
	public:
		explicit Image(const std::string &filename);

		void *GetImTextureID();
		vector2f GetSize();
		vector2f GetUv();

	private:
		RefCountedPtr<Graphics::Texture> m_texture;
	};

} // namespace PiGui

#endif
