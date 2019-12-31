// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "graphics/TextureBuilder.h"

namespace PiGUI {

	RefCountedPtr<Graphics::Material> Face::s_material;

	Face::Face(FaceParts::FaceDescriptor &face, Uint32 seed)
	{
		PROFILE_SCOPED()
		if (!seed) seed = time(0);

		m_seed = seed;

		SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FaceParts::FACE_WIDTH, FaceParts::FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

		FaceParts::PickFaceParts(face, m_seed);
		FaceParts::BuildFaceImage(faceim.Get(), face);

		m_texture.Reset(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).GetOrCreateTexture(Pi::renderer, std::string("face")));

		if (!s_material) {
			Graphics::MaterialDescriptor matDesc;
			matDesc.textures = 1;
			s_material.Reset(Pi::renderer->CreateMaterial(matDesc));
		}
	}

	Uint32 Face::GetTextureId()
	{
		return m_texture->GetTextureID();
	}

	vector2f Face::GetTextureSize()
	{
		return m_texture->GetDescriptor().texSize;
	}

} // namespace PiGUI
