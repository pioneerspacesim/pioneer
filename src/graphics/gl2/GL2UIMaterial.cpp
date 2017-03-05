// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2UIMaterial.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "GL2Texture.h"
#include "GL2Renderer.h"
#include <sstream>
#include "StringF.h"

namespace Graphics
{
	namespace GL2
	{
		///////////////////////////////////////////////////////////////////////
		UIProgram::UIProgram(const MaterialDescriptor &desc)
		{
			m_name = "ui";
			RENDERER_CHECK_ERRORS();

			LoadShaders(m_name, m_defines);
			InitUniforms();
		}

		Program *UIMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			return new UIProgram(desc);
		}

		void UIMaterial::Apply()
		{
			GL2::Material::Apply();

			UIProgram *p = static_cast<UIProgram*>(m_program);

			p->diffuse.Set(this->diffuse);

			p->texture0.Set(this->texture0, 0);
			p->texture1.Set(this->texture1, 1);
		}

		void UIMaterial::Unapply()
		{
			// Might not be necessary to unbind textures, but let's not old graphics code (eg, old-UI)
			if ( texture1 ) {
				static_cast<GL2Texture*>(texture1)->Unbind();
				glActiveTexture(GL_TEXTURE0);
			}
			if ( texture0 ) {
				static_cast<GL2Texture*>(texture0)->Unbind();
			}
		}
	}
}
