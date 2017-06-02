// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_MATERIAL_H
#define _GL2_MATERIAL_H
/*
 * Multi-purpose GL2 material.
 *
 * Generally, the idea is that a Program contains uniforms but
 * a material sets them, using the standard parameters of Graphics::Material
 * or whatever necessary to achieve an effect
 *
 * Programs are owned by the renderer, since they are shared between materials.
 */
#include "libs.h"
#include "graphics/Material.h"

namespace Graphics {

	class RendererGL2;

	namespace GL2 {

		class Program;

		class Material : public Graphics::Material {
		public:
			Material() { }
			// Create an appropriate program for this material.
			virtual Program *CreateProgram(const MaterialDescriptor &) = 0;
			// bind textures, set uniforms
			virtual void Apply() override;
			virtual void Unapply() override;
			virtual void SetProgram(Program *p) { m_program = p; }
			virtual bool IsProgramLoaded() const override final { return true; }
			virtual void SetCommonUniforms(const matrix4x4f& mv, const matrix4x4f& proj) override final;

		protected:
			friend class Graphics::RendererGL2;
			Program *m_program;
			RendererGL2 *m_renderer;
		};
	}
}
#endif
