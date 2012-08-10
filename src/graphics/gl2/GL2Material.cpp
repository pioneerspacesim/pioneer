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
