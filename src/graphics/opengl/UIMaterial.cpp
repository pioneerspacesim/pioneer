// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UIMaterial.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "TextureGL.h"
#include "RendererGL.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
	namespace OGL {
		///////////////////////////////////////////////////////////////////////
		UIProgram::UIProgram(const MaterialDescriptor &desc)
		{
			m_name = "ui";
			CHECKERRORS();

			LoadShaders(m_name, m_defines);
			InitUniforms();
		}

		Program *UIMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			return new UIProgram(desc);
		}

		void UIMaterial::Apply()
		{
			OGL::Material::Apply();

			UIProgram *p = static_cast<UIProgram*>(m_program);

			p->diffuse.Set(this->diffuse);

			p->texture0.Set(this->texture0, 0);
			p->texture1.Set(this->texture1, 1);
		}

		void UIMaterial::Unapply()
		{
			// Might not be necessary to unbind textures, but let's not old graphics code (eg, old-UI)
			if ( texture1 ) {
				static_cast<TextureGL*>(texture1)->Unbind();
				glActiveTexture(GL_TEXTURE0);
			}
			if ( texture0 ) {
				static_cast<TextureGL*>(texture0)->Unbind();
			}
		}
	}
}
