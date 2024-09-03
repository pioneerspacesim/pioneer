// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Renderer.h"
#include "Texture.h"
#include "jenkins/lookup3.h"

#include <SDL.h>

namespace Graphics {

	Renderer::Renderer(SDL_Window *window, int w, int h) :
		m_width(w),
		m_height(h),
		m_ambient(Color::BLACK),
		m_window(window)
	{
	}

	Renderer::~Renderer()
	{
		RemoveAllCachedTextures();
		SDL_DestroyWindow(m_window);
	}

	Texture *Renderer::GetCachedTexture(const std::string &type, const std::string &name)
	{
		TextureCacheMap::iterator i = m_textureCache.find(TextureCacheKey(type, name));
		if (i == m_textureCache.end()) return 0;
		return (*i).second->Get();
	}

	void Renderer::AddCachedTexture(const std::string &type, const std::string &name, Texture *texture)
	{
		RemoveCachedTexture(type, name);
		m_textureCache.insert(std::make_pair(TextureCacheKey(type, name), new RefCountedPtr<Texture>(texture)));
	}

	void Renderer::RemoveCachedTexture(const std::string &type, const std::string &name)
	{
		TextureCacheMap::iterator i = m_textureCache.find(TextureCacheKey(type, name));
		if (i == m_textureCache.end()) return;
		delete (*i).second;
		m_textureCache.erase(i);
	}

	void Renderer::RemoveAllCachedTextures()
	{
		for (TextureCacheMap::iterator i = m_textureCache.begin(); i != m_textureCache.end(); ++i)
			delete (*i).second;
		m_textureCache.clear();
	}

} // namespace Graphics
