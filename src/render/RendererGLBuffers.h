#ifndef _RENDERER_GL_BUFFERS_H
#define _RENDERER_GL_BUFFERS_H

/* OpenGL renderer data structures and bufferobject stuff.
 * This can be used by both the Legacy and GL2 renderers
 */

struct Vertex {
	vector3f position;
};

//+color
//Users: background
struct UnlitVertex : public Vertex {
	Color color;
};

//+normal, uv0
//Users: LMRModel
struct ModelVertex : public Vertex {
	vector3f normal;
	vector2f uv;
};

//under restructuring
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

	virtual ~Buffer() {
		glDeleteBuffers(1, &m_buf);
	}

	virtual void Bind() {
		glBindBuffer(m_target, m_buf);
	}

	virtual void Unbind() {
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

	virtual void Draw(GLenum pt, int start, int count) { }
};

//array or element array buffer base class
//(think "vertex buffer" & "index buffer")
class BufferBase {
public:
	BufferBase(GLenum target, unsigned int maxElements) :
		m_maxSize(maxElements),
		m_target(target),
		m_usageHint(GL_STATIC_DRAW),
		m_offset(0)
	{
		glGenBuffers(1, &m_buf);
	}

	virtual ~BufferBase() {
		glDeleteBuffers(1, &m_buf);
	}

	void Bind() {
		glBindBuffer(m_target, m_buf);
	}

	void Unbind() {
		glBindBuffer(m_target, 0);
	}

	template<typename T>
	int BufferData(const int count, const T *v) {
		//initialise data store on first use
		assert(count > 0);
		if (m_offset == 0) {
			glBufferData(m_target, sizeof(T)*m_maxSize, 0, m_usageHint);
		}
		int current = m_offset;
		glBufferSubData(m_target, sizeof(T)*m_offset, sizeof(T)*count, v);
		m_offset += count;
		return current;
	}

private:
	GLenum m_usageHint;
	GLenum m_target;
	int m_offset;
	unsigned int m_maxSize;
	unsigned int m_buf;
};

class IndexBuffer : public BufferBase {
public:
	IndexBuffer(unsigned int maxElements) :
		BufferBase(GL_ELEMENT_ARRAY_BUFFER, maxElements)
	{
	}

	int BufferIndexData(const int count, const unsigned short *v) {
		return BufferData<unsigned short>(count, v);
	}
};

class VertexBuffer : public BufferBase {
public:
	VertexBuffer(unsigned int maxElements) :
		BufferBase(GL_ARRAY_BUFFER, maxElements)
	{
		m_states[0] = GL_VERTEX_ARRAY;
		m_states[1] = GL_NORMAL_ARRAY;
		m_states[2] = GL_TEXTURE_COORD_ARRAY;
		m_numstates = 3;
	}

	virtual void Draw(GLenum pt, unsigned int start, unsigned int count) {
		EnableClientStates();
		SetPointers();
		glDrawArrays(pt, start, count);
		DisableClientStates();
	};

	virtual void DrawIndexed(GLenum pt, unsigned int start, unsigned int count) {
		EnableClientStates();
		SetPointers();
		//XXX use DrawRangeElements for potential performance boost
		glDrawElements(pt, count, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(start*sizeof(GLushort)));
		DisableClientStates();
	}

	//make things nicer to read
	void VertexPointer(GLsizei stride, size_t pointer) {
		glVertexPointer(3, GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(pointer));
	}

	void NormalPointer(GLsizei stride, size_t pointer) {
		glNormalPointer(GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(pointer));
	}

	void TexCoordPointer(GLsizei stride, size_t pointer) {
		glTexCoordPointer(2, GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(pointer));
	}

	void ColorPointer(GLsizei stride, size_t pointer) {
		glColorPointer(4, GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(pointer));
	}

	//XXX this only supports LMR vertices!!
	virtual void SetPointers() {
		VertexPointer(sizeof(ModelVertex), offsetof(ModelVertex, position));
		NormalPointer(sizeof(ModelVertex), offsetof(ModelVertex, normal));
		TexCoordPointer(sizeof(ModelVertex), offsetof(ModelVertex, uv));
	}

	virtual void EnableClientStates() {
		for (int i=0; i < m_numstates; i++)
			glEnableClientState(m_states[i]);
	}

	virtual void DisableClientStates() {
		for (int i=0; i < m_numstates; i++)
			glDisableClientState(m_states[i]);
	}

protected:
	GLenum m_states[3];
	int m_numstates;
};

class UnlitVertexBuffer : public VertexBuffer {
public:
	UnlitVertexBuffer(unsigned int maxElements) : VertexBuffer(maxElements) {
		m_states[0] = GL_VERTEX_ARRAY;
		m_states[1] = GL_COLOR_ARRAY;
		m_numstates = 2;
	}

	virtual void SetPointers() {
		VertexPointer(sizeof(UnlitVertex), offsetof(UnlitVertex, position));
		ColorPointer(sizeof(UnlitVertex), offsetof(UnlitVertex, color));
	}
};

#endif
