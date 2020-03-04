// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "FileSystem.h"
#include "graphics/TextureBuilder.h"

namespace PiGUI {

	Image::Image(const std::string &filename)
	{
		m_texture.Reset(Graphics::TextureBuilder::Model(filename).GetOrCreateTexture(Pi::renderer, "model"));
	}

	Uint32 Image::GetId()
	{
		return m_texture->GetTextureID();
	}

	vector2f Image::GetSize()
	{
		auto size = m_texture->GetDescriptor().dataSize;
		return vector2f(size.x, size.y);
	}

	vector2f Image::GetUv()
	{
		return m_texture->GetDescriptor().texSize;
	}

} // namespace PiGUI
