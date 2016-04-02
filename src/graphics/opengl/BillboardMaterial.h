// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_BILLBOARDMATERIAL_H
#define _OGL_BILLBOARDMATERIAL_H
/*
 * point sprite (aka billboard) material
 */
#include "OpenGLLibs.h"
#include "MaterialGL.h"
#include "Program.h"
namespace Graphics {

	namespace OGL {

		class BillboardMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			void Apply();
			void Unapply();
		};
	}
}
#endif
