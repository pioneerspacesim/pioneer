// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MultiMaterial.h"

#include "HeatGradientPar.h"
#include "RendererGL.h"
#include "StringF.h"
#include "TextureGL.h"
#include "core/Log.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Types.h"
#include <sstream>

namespace Graphics {
	namespace OGL {

		static size_t s_lightIntensity = Renderer::GetName("lightIntensity");
		static size_t s_heatingNormal = Renderer::GetName("heatingNormal");
		static size_t s_heatGradient = Renderer::GetName("heatGradient");
		static size_t s_tex4name = Renderer::GetName("texture4");
		static size_t s_tex5name = Renderer::GetName("texture5");

		LitMultiMaterial::LitMultiMaterial() :
			m_curNumLights(0)
		{
		}

		Shader *MultiMaterial::CreateShader(const MaterialDescriptor &desc)
		{
			Shader *s = new Shader("multi", desc);
			Log::Info("Initializing MultiShader Uniforms for shader {}\n", (void *)this);

			s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture1", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture2", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture3", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture4", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture5", TextureType::TEXTURE_2D);
			s->AddTextureBinding("texture6", TextureType::TEXTURE_2D);
			s->AddTextureBinding("heatGradient", TextureType::TEXTURE_2D);

			s->AddConstantBinding("lightIntensity", ConstantDataFormat::DATA_FORMAT_FLOAT4);
			s->AddConstantBinding("heatingNormal", ConstantDataFormat::DATA_FORMAT_FLOAT4);
			return s;
		}

		void MultiMaterial::Apply()
		{
			SetTexture(s_tex4name, texture4);
			SetTexture(s_tex5name, texture5);

			float intensity[4] = { 0.f, 0.f, 0.f, 0.f };
			for (uint32_t i = 0; i < m_renderer->GetNumLights(); i++) {
				intensity[i] = m_renderer->GetLight(i).GetIntensity();
			}

			SetPushConstant(s_lightIntensity, Color4f(intensity[0], intensity[1], intensity[2], intensity[3]));

			OGL::Material::Apply();
		}

		void LitMultiMaterial::Apply()
		{
			uint32_t numLights = m_renderer->GetNumLights();

			//request a new light variation
			// TODO: need a slightly better (and more generic) way to access loaded programs and generate state variations
			if (m_curNumLights != numLights) {
				MaterialDescriptor desc = GetDescriptor();
				m_curNumLights = desc.dirLights = numLights;
				Program *p = m_shader->GetProgramForDesc(desc);
				if (p->Loaded())
					m_activeVariant = p;
			}

			MultiMaterial::Apply();

			CHECKERRORS();
		}

	} // namespace OGL
} // namespace Graphics
