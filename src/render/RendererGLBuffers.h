#ifndef _RENDERER_GL_BUFFERS_H
#define _RENDERER_GL_BUFFERS_H

/* OpenGL renderer data structures and bufferobject stuff.
 * This can be used by both the Legacy and GL2 renderers
 */

struct Vertex {

};

//position, color.
struct UnlitVertex : public Vertex {
	vector3f position;
	Color color;
};

struct ModelVertex : public Vertex {
	vector3f position;
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

class BufferBase {
public:
	BufferBase(unsigned int target) :
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
	int BufferData(int count, const T *v) {
		//initialise data store!!
		glBufferData(m_target, sizeof(T)*count, v, m_usageHint);
		//glBufferSubData(m_target, m_offset, count*sizeof(T), v);
		m_offset += count * sizeof(T);
		return count;
	}

private:
	unsigned int m_buf;
	unsigned int m_target;
	int m_offset;
	GLenum m_usageHint;
};

class IndexBuffer : public BufferBase {
public:
	IndexBuffer() :
		BufferBase(GL_ELEMENT_ARRAY_BUFFER)
	{
	}

	int BufferIndexData(int count, const unsigned short *v) {
		return BufferData<unsigned short>(count, v);
	}
};

//vertex buffer?
class VertexBuffer : public BufferBase {
public:
	VertexBuffer() :
		BufferBase(GL_ARRAY_BUFFER)
	{
		m_states[0] = GL_VERTEX_ARRAY;
		m_states[1] = GL_NORMAL_ARRAY;
		m_states[2] = GL_TEXTURE_COORD_ARRAY;
		m_numstates = 3;
	}

	virtual void Draw(unsigned int start, unsigned int count) {
		EnableClientStates();
		SetPointers();
		const GLenum pt = GL_TRIANGLES;
		glDrawArrays(pt, start, count);
		DisableClientStates();
	};

	virtual void DrawIndexed(unsigned int start, unsigned int count) {
		EnableClientStates();
		SetPointers();
		const GLenum pt = GL_TRIANGLES;
		glDrawElements(pt, count, GL_UNSIGNED_SHORT, 0);
		DisableClientStates();
	}

	virtual void SetPointers() {
		glVertexPointer(3, GL_FLOAT, sizeof(ModelVertex), reinterpret_cast<const GLvoid *>(offsetof(ModelVertex, position)));
		glNormalPointer(GL_FLOAT, sizeof(ModelVertex), reinterpret_cast<const GLvoid *>(offsetof(ModelVertex, normal)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ModelVertex), reinterpret_cast<const GLvoid *>(offsetof(ModelVertex, uv)));
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

#endif
