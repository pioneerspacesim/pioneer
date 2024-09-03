// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "FileSystem.h"
#include "graphics/TextureBuilder.h"

namespace PiGui {

	Image::Image(const std::string &filename)
	{
		m_texture.Reset(Graphics::TextureBuilder::UI(filename).GetOrCreateTexture(Pi::renderer, "ui"));
	}

	void *Image::GetImTextureID()
	{
		return m_texture.Get();
	}

	vector2f Image::GetSize()
	{
		auto size = m_texture->GetDescriptor().dataSize;
		return vector2f(size.x, size.y) * GetUv();
	}

	vector2f Image::GetUv()
	{
		return m_texture->GetDescriptor().texSize;
	}

} // namespace PiGui
