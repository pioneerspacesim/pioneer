#include "RendererGL2.h"
#include "Material.h"
#include "Render.h"
#include "Texture.h"

template <typename T>
class Buffer {
private:
	GLuint m_buf;
	int m_numverts;
	int m_size;
	unsigned int m_target;

public:
	Buffer(int size) : m_numverts(0), m_size(size) {
		glGenBuffers(1, &m_buf);
		m_target = GL_ARRAY_BUFFER;
		Bind();
		// create data storage
		glBufferData(m_target, sizeof(T)*size, 0, GL_STREAM_DRAW);
		Unbind();
	}

	~Buffer() {
		glDeleteBuffers(1, &m_buf);
	}

	void Bind() {
		glBindBuffer(m_target, m_buf);
	}

	void Unbind() {
		glBindBuffer(m_target, 0);
	}

	void Reset() {
		m_numverts = 0;
	}

	int BufferData(int count, const T *v) {
		assert(m_numverts += count <= m_size);
		glBufferSubData(m_target, m_numverts*sizeof(T), count*sizeof(T), v);
		int start = m_numverts;
		m_numverts += count;
		return start;
	}

	void Draw(GLenum pt, int start, int count) {

	}
};

template<> void Buffer<LineVertex>::Draw(GLenum pt, int start, int count) {
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), reinterpret_cast<const GLvoid *>(offsetof(LineVertex, position)));
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), reinterpret_cast<const GLvoid *>(offsetof(LineVertex, color)));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(pt, start, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

Render::Shader *simpleTextured;
Render::Shader *flatProg;
Buffer<LineVertex> *lineBuffer;

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{
	simpleTextured = new Render::Shader("simpleTextured");
	flatProg = new Render::Shader("flat");
	lineBuffer = new Buffer<LineVertex>(1000);
}

RendererGL2::~RendererGL2()
{
	delete simpleTextured;
	delete flatProg;
	delete lineBuffer;
}

bool RendererGL2::BeginFrame()
{
	lineBuffer->Reset();
	Render::PrepareFrame();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool RendererGL2::SetPerspectiveProjection(float fov, float aspect, float near, float far)
{
	double ymax = near * tan(fov * M_PI / 360.0);
	double ymin = -ymax;
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(xmin, xmax, ymin, ymax, near, far);

	// update values for log-z hack
	Render::State::SetZnearZfar(near, far);
	return true;
}

bool RendererGL2::DrawLines(int count, const LineVertex *v, LineType t)
{
	if (count < 2 || !v) return false;

	Render::State::UseProgram(Render::simpleShader);
	lineBuffer->Bind();
	//since it's drawn immediately, there isn't much point in having the offset
	int offset = lineBuffer->BufferData(count, v);
	lineBuffer->Draw(t, offset, count);
	lineBuffer->Unbind();
	Render::State::UseProgram(0);

	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	Render::State::UseProgram(flatProg);
	flatProg->SetUniform4f("color", c);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	Render::State::UseProgram(0);

	return true;
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
	const bool flat = (m && v->diffuse.empty());

	if (twoSided)
		glDisable(GL_CULL_FACE);

	//program choice
	Render::Shader *s = 0;
	if (!m)
		s = Render::simpleShader;
	else if (m->shader)
		s = m->shader;
	else if (flat)
		s = flatProg;
	else if (textured)
		s = simpleTextured;
	if (s)
		Render::State::UseProgram(s);

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
		assert(v->uv0.size() == v->position.size());
		m->texture0->Bind();
		s->SetUniform1i("texture0", 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}
	if (flat) {
		assert(diffuse == false);
		s->SetUniform4f("color", m->diffuse);
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
	if (s)
		Render::State::UseProgram(0);

	return true;
}
