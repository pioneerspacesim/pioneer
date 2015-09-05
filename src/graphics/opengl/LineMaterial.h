// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_LINEMATERIAL_H
#define _OGL_LINEMATERIAL_H
/*
 * Programs & Materials used by terrain
 */
#include "libs.h"
#include "MaterialGL.h"
#include "Program.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
	namespace OGL {
		struct LineMaterialParams {
			float lineWidth;
		};

		class LineProgram : public Program {
		public:
			LineProgram(const std::string &filename, const std::string &defines);

			Uniform lineWidth;

		protected:
			virtual void InitUniforms();
		};

		class LineMaterial : public Material {
			virtual Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();

		protected:
			void SetGSUniforms();
		};
	}
}
#endif
