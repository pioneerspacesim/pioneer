// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_BILLBOARDMATERIAL_H
#define _OGL_BILLBOARDMATERIAL_H
/*
 * point sprite (aka billboard) material
 */

#include "MaterialGL.h"
#include "Program.h"
#include "graphics/Renderer.h"

namespace Graphics {

	namespace OGL {

		class BillboardMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				assert(desc.textures == 1);
				Shader *s = new Shader("billboards", desc);
				m_tex0name = s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
				m_coordScaleName = s->AddConstantBinding("coordDownScale", ConstantDataFormat::DATA_FORMAT_FLOAT);
				return s;
			}

			virtual void Apply() override
			{
				float coordDownScale = 0.5f;
				if (this->specialParameter0)
					coordDownScale = *static_cast<float *>(this->specialParameter0);

				SetTexture(m_tex0name, texture0);
				SetPushConstant(m_coordScaleName, coordDownScale);

				Material::Apply();
			}

		private:
			size_t m_tex0name;
			size_t m_coordScaleName;
		};
	} // namespace OGL
} // namespace Graphics
#endif
