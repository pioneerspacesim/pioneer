// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_MATERIAL_H
#define _OGL_MATERIAL_H
/*
 * Multi-purpose OGL material.
 *
 * Generally, the idea is that a Program contains uniforms but
 * a material sets them, using the standard parameters of Graphics::Material
 * or whatever necessary to achieve an effect
 *
 * Programs are owned by the renderer, since they are shared between materials.
 */
#include "OpenGLLibs.h"
#include "graphics/Material.h"

namespace Graphics {

	class RendererOGL;

	namespace OGL {

		class Program;

		class Material : public Graphics::Material {
		public:
			Material() { }
			// Create an appropriate program for this material.
			virtual Program *CreateProgram(const MaterialDescriptor &) = 0;
			// bind textures, set uniforms
			virtual void Apply() override;
			virtual void Unapply() override;
			virtual bool IsProgramLoaded() const override final;
			virtual void SetProgram(Program *p) { m_program = p; }
			virtual void SetCommonUniforms(const matrix4x4f& mv, const matrix4x4f& proj) override;

		protected:
			friend class Graphics::RendererOGL;
			Program *m_program;
			RendererOGL *m_renderer;
		};
	}
}
#endif
