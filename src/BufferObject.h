#ifndef BUFFEROBJECT_H
#define BUFFEROBJECT_H

#include "graphics/Graphics.h"

// 2MiB @ 32 bytes/vertex
#define VERTICES_IN_BUFFER 65536

template <int VERTEX_SIZE>
class BufferObject {
public:
	BufferObject() {
		m_elementsTempDirty = false;
		m_vertexPos = 0;
		glGenBuffersARB(1, &m_vertexArrayBufferObject);
		glGenBuffersARB(1, &m_elementArrayBufferObject);
		Graphics::BindArrayBuffer(m_vertexArrayBufferObject);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, VERTICES_IN_BUFFER * VERTEX_SIZE, 0, GL_DYNAMIC_DRAW);
		Graphics::BindArrayBuffer(0);
	}

	~BufferObject() {
		glDeleteBuffersARB(1, &m_vertexArrayBufferObject);
		glDeleteBuffersARB(1, &m_elementArrayBufferObject);
	}

	int GetVertexSpaceLeft() {
		return VERTICES_IN_BUFFER - m_vertexPos;
	}

	/** Returns new index base for index data */
	int AddGeometry(int numVertices, void *vtxData, int numIndices, Uint16 *idxData) {
		assert(GetVertexSpaceLeft() >= numVertices);

		Graphics::BindArrayBuffer(m_vertexArrayBufferObject);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, m_vertexPos*VERTEX_SIZE, 
			numVertices * VERTEX_SIZE, vtxData);
		Graphics::BindArrayBuffer(0);

		const int indexBase = m_elementsTemp.size();
		for (int i=0; i<numIndices; i++) {
			m_elementsTemp.push_back(m_vertexPos + idxData[i]);
		}
		m_vertexPos += numVertices;
		m_elementsTempDirty = true;
		return indexBase;
	}

	void BindBuffersForDraw() {
		FlushElementsArray();
		if ( (!Graphics::IsArrayBufferBound(m_vertexArrayBufferObject)) ||
		     (!Graphics::IsElementArrayBufferBound(m_elementArrayBufferObject)) ) {
			Graphics::BindArrayBuffer(m_vertexArrayBufferObject);
			glNormalPointer(GL_FLOAT, VERTEX_SIZE, reinterpret_cast<void *>(3*sizeof(float)));
			glVertexPointer(3, GL_FLOAT, VERTEX_SIZE, 0);
			glTexCoordPointer(2, GL_FLOAT, VERTEX_SIZE, reinterpret_cast<void *>(2*3*sizeof(float)));
			Graphics::BindElementArrayBuffer(m_elementArrayBufferObject);
		}
	}
private:
	void FlushElementsArray() {
		if (m_elementsTempDirty) {
			Graphics::BindElementArrayBuffer(m_elementArrayBufferObject);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(Uint16)*m_elementsTemp.size(),
					&m_elementsTemp[0], GL_DYNAMIC_DRAW);
			Graphics::BindElementArrayBuffer(0);
			m_elementsTempDirty = false;
		}
	}
	GLuint m_vertexArrayBufferObject;
	GLuint m_elementArrayBufferObject;
	bool m_elementsTempDirty;
	std::vector<Uint16> m_elementsTemp;
	int m_vertexPos;
};

template <int VERTEX_SIZE>
class BufferObjectPool {
public:
	BufferObjectPool() {

	}
	~BufferObjectPool() {
		for (int i=0; i < int(m_bos.size()); i++) delete m_bos[i];
	}

	void AddGeometry(int numVertices, void *vtxData, int numIndices, Uint16 *idxData,
			int *outIndexBase, BufferObject<VERTEX_SIZE> **outBufferObject)
	{
		BufferObject<VERTEX_SIZE> *bo = 0;
		//int boIdx = -1;
		for (unsigned int i=0; i<m_bos.size(); i++) {
			if (m_bos[i]->GetVertexSpaceLeft() >= numVertices) {
				bo = m_bos[i];
		//		boIdx = i;
			}
		}
		if (bo == 0) {
			bo = new BufferObject<VERTEX_SIZE>();
		//	boIdx = m_bos.size();
			m_bos.push_back(bo);
		}

		*outBufferObject = bo;
		*outIndexBase = bo->AddGeometry(numVertices, vtxData, numIndices, idxData);
		//printf("Base %d in buffer number %d\n", *outIndexBase, boIdx);
	}
private:
	std::vector<BufferObject<VERTEX_SIZE>* > m_bos;
};

#endif /* BUFFEROBJECT_H */
