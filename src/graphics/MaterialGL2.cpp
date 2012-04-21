#include "MaterialGL2.h"
#include "Shader.h"
#include "TextureGL.h"

namespace Graphics {

void MaterialGL2::Apply() const
{
	shader->Use();
	shader->SetUniform("material.diffuse", diffuse);
	shader->SetUniform("material.specular", specular);
	shader->SetUniform("material.emissive", emissive);
	shader->SetUniform("material.shininess", shininess);
	shader->SetUniform("scene.ambient", Color(0.f));
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Bind();
		shader->SetUniform("texture0", 0);
	}
	if (texture1) {
		glActiveTexture(GL_TEXTURE1);
		static_cast<TextureGL*>(texture1)->Bind();
		shader->SetUniform("texture1", 1);
	}
	if (texture2) {
		glActiveTexture(GL_TEXTURE2);
		static_cast<TextureGL*>(texture2)->Bind();
		shader->SetUniform("texture2", 2);
	}
}

void MaterialGL2::Unapply() const
{
	glActiveTexture(GL_TEXTURE0);
	shader->Unuse();
}

}