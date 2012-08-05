#include "MultiMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include <sstream>

namespace Graphics {
namespace GL2 {

MultiProgram::MultiProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;
	if (desc.texture > 0)
		ss << "#define TEXTURE0 1\n";

	m_name = "multi";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void MultiProgram::InitUniforms()
{
	texture0.Init("texture0", m_program);
}

void MultiMaterial::Apply()
{
	MultiProgram *p = static_cast<MultiProgram*>(m_program);
	p->Use();

	//set some uniforms
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Bind();
		p->texture0.Set(0);
	}
}

void MultiMaterial::Unapply()
{
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Unbind();
	}
	m_program->Unuse();
}

}
}
