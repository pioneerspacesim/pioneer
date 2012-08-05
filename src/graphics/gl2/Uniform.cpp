#include "Uniform.h"

namespace Graphics {
namespace GL2 {

Uniform::Uniform()
: m_location(-1)
{
}

void Uniform::Init(const char *name, GLuint program)
{
	m_location = glGetUniformLocation(program, name);
}

void Uniform::Set(int i)
{
	if (m_location != -1)
		glUniform1i(m_location, i);
}

void Uniform::Set(float f)
{
	if (m_location != -1)
		glUniform1f(m_location, f);
}

void Uniform::Set(const Color4f &c)
{
	if (m_location != -1)
		glUniform4f(m_location, c.r, c.g, c.b, c.a);
}

}
}
