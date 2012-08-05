#ifndef _GL2_MATERIAL_H
#define _GL2_MATERIAL_H
/*
 * Multi-purpose GL2 material.
 * Sets program parameters
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
			virtual void Apply();
			virtual void Unapply();

		protected:
			friend class RendererGL2;
			Program *m_program;
		};
	}
}
#endif
