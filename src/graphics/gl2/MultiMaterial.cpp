#include "MultiMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>

namespace Graphics {
namespace GL2 {

MultiProgram::MultiProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";
	if (desc.vertexColors)
		ss << "#define VERTEXCOLOR\n";
	if (desc.alphaTest)
		ss << "#define ALPHA_TEST\n";

	m_name = "multi";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

Program *MultiMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new MultiProgram(desc);
}

void MultiMaterial::Apply()
{
	MultiProgram *p = static_cast<MultiProgram*>(m_program);
	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);
	p->diffuse.Set(this->diffuse);

	//set some uniforms
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Bind();
		p->texture0.Set(0);
	}

	glPushAttrib(GL_ENABLE_BIT);
	if (this->twoSided)
		glDisable(GL_CULL_FACE);
}

void MultiMaterial::Unapply()
{
	glPopAttrib();
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Unbind();
	}
	m_program->Unuse();
}

}
}
