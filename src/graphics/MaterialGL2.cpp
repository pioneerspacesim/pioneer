#include "MaterialGL2.h"
#include "Shader.h"
#include "TextureGL.h"

namespace Graphics {

void MaterialGL2::Apply() const
{
	shader->Use();
	shader->SetUniform("color", Color(1.f, 0.f, 0.3f, 1.f));
	shader->SetUniform("diffuse", diffuse);
	shader->SetUniform("specular", specular);
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Bind();
		shader->SetUniform("texture0", 0);
	}
	if (texture1) {
		glActiveTexture(GL_TEXTURE1);
		static_cast<TextureGL*>(texture1)->Bind();
		shader->SetUniform("texture1", 1);
	}
}

void MaterialGL2::Unapply() const
{
	glActiveTexture(GL_TEXTURE0);
	shader->Unuse();
}

}