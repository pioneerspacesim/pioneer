// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2Material.h"
#include "Program.h"
#include "graphics/RendererGL2.h"

namespace Graphics {
namespace GL2 {

void Material::Apply()
{
	m_program->Use();
}

void Material::Unapply()
{
	m_program->Unuse();
}

}
}
