#include "Material.h"
#include "Program.h"

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
