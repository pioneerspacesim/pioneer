#ifndef _RENDERER_GL_BUFFERS_H
#define _RENDERER_GL_BUFFERS_H

#include "libs.h"

namespace Graphics {

/* OpenGL renderer data structures and bufferobject stuff.
 * This can be used by both the Legacy and GL2 renderers
 */

struct GLVertex {
	vector3f position;
};

//+color
//Users: background
struct UnlitVertex /*: public GLVertex*/ {
	vector3f position;
	Color color;
};

//+normal, uv0
//Users: LMRModel
struct ModelVertex /*: public GLVertex*/ {
	vector3f position;
	vector3f normal;
	vector2f uv;
};

//array or element array buffer base class
//(think "vertex buffer" & "index buffer")
class BufferBase {
public:
	BufferBase(GLenum target, unsigned int maxElements) :
		m_target(target),
		m_usageHint(GL_STATIC_DRAW),
		m_offset(0),
		m_maxSize(maxElements)
	{
		glGenBuffersARB(1, &m_buf);
	}

	virtual ~BufferBase() {
		glDeleteBuffersARB(1, &m_buf);
	}

	void Bind() {
		glBindBufferARB(m_target, m_buf);
	}

	void Unbind() {
		glBindBufferARB(m_target, 0);
	}

	template<typename T>
	int BufferData(const int count, const T *v) {
		//initialise data store on first use
		assert(count > 0);
		if (m_offset == 0) {
			glBufferDataARB(m_target, sizeof(T)*m_maxSize, 0, m_usageHint);
		}
		int current = m_offset;
		glBufferSubDataARB(m_target, sizeof(T)*m_offset, sizeof(T)*count, v);
		m_offset += count;
		return current;
	}

private:
	GLenum m_target;
	GLenum m_usageHint;
	int m_offset;
	unsigned int m_buf;
	unsigned int m_maxSize;
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

}

#endif
