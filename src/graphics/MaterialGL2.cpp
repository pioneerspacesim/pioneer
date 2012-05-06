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
		glActiveTexture(GL_TEXTURE0 + 1);
		static_cast<TextureGL*>(texture1)->Bind();
		shader->SetUniform("texture1", 1);
	}
	if (texture2) {
		glActiveTexture(GL_TEXTURE0 + 2);
		static_cast<TextureGL*>(texture2)->Bind();
		shader->SetUniform("texture2", 2);
	}
	if (texture3) {
		glActiveTexture(GL_TEXTURE0 + 3);
		static_cast<TextureGL*>(texture3)->Bind();
		shader->SetUniform("texture3", 3);
	}
	if (texture4) {
		glActiveTexture(GL_TEXTURE0 + 4);
		static_cast<TextureGL*>(texture4)->Bind();
		shader->SetUniform("texture4", 4);
	}
}

void MaterialGL2::Unapply() const
{
	glActiveTexture(GL_TEXTURE0);
	shader->Unuse();
}

}