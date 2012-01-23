#include "RendererGL2.h"
#include "Render.h"
#include "Texture.h"

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{

}

RendererGL2::~RendererGL2()
{

}

// XXX copypaste from legacy renderer
bool RendererGL2::DrawTriangles(const VertexArray *v, const Material *m, PrimitiveType t)
{
	if (!v || v->position.size() < 3) return false;

	const bool diffuse = !v->diffuse.empty();
	const bool textured = (m && m->texture0 && v->uv0.size() == v->position.size());
	const bool normals = !v->normal.empty();
	const unsigned int numverts = v->position.size();
	const bool twoSided = (m && m->twoSided);

	if (twoSided)
		glDisable(GL_CULL_FACE);

	//program choice
	if (!m)
		Render::State::UseProgram(Render::simpleShader);
	else if(m->type == Material::TYPE_PLANETRING) {
		const int lights = Clamp(m_numLights-1, 0, 3);
		Render::State::UseProgram(Render::planetRingsShader[lights]);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));
	if (diffuse) {
		assert(v->diffuse.size() == v->position.size());
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));
	}
	if (normals) {
		assert(v->normal.size() == v->position.size());
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
	}
	if (textured) {
		//XXX need a simple texturedShader
		assert(v->uv0.size() == v->position.size());
		m->texture0->Bind();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}

	glDrawArrays(t, 0, numverts);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (diffuse)
		glDisableClientState(GL_COLOR_ARRAY);
	if (normals)
		glDisableClientState(GL_NORMAL_ARRAY);
	if (textured) {
		m->texture0->Unbind();
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (twoSided)
		glEnable(GL_CULL_FACE);

	// XXX won't be necessary
	if (!m)
		Render::State::UseProgram(0);
	else if(m->type == Material::TYPE_PLANETRING)
		Render::State::UseProgram(0);

	return true;
}
