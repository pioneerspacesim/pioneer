#ifndef _VERTEXARRAY_H
#define _VERTEXARRAY_H

#include "libs.h"

namespace Graphics {

//allowed minimum of GL_MAX_VERTEX_ATTRIBS is 8 on ES2
//XXX could implement separate position2D, position3D
enum VertexAttrib {
	ATTRIB_POSITION  = (1u << 0),
	ATTRIB_NORMAL    = (1u << 1),
	ATTRIB_DIFFUSE   = (1u << 2),
	ATTRIB_UV0       = (1u << 3),
	//ATTRIB_UV1       = (1u << 4),
	//ATTRIB_TANGENT   = (1u << 5),
	//ATTRIB_BITANGENT = (1u << 6),
	//ATTRIB_CUSTOM?
};

typedef unsigned int AttributeSet;

/*
 * VertexArray is a multi-purpose vertex container. Users specify
 * the attributes they intend to use and then add vertices. Renderers
 * do whatever they need to do with regards to the attribute set.
 * This is not optimized for high performance drawing, but okay for simple
 * cases.
 */
class VertexArray {
public:
	//specify attributes to be used, additionally reserve space for vertices
	VertexArray(AttributeSet attribs, int size=0);
	virtual ~VertexArray();

	//check presence of an attribute
	virtual bool HasAttrib(VertexAttrib v) const;
	virtual unsigned int GetNumVerts() const;
	virtual AttributeSet GetAttributeSet() const { return m_attribs; }

	//removes vertices, does not deallocate space
	virtual void Clear();

	// don't mix these
	virtual void Add(const vector3f &v);
	virtual void Add(const vector3f &v, const Color &c);
	virtual void Add(const vector3f &v, const Color &c, const vector3f &normal);
	virtual void Add(const vector3f &v, const Color &c, const vector2f &uv);
	virtual void Add(const vector3f &v, const vector2f &uv);
	virtual void Add(const vector3f &v, const vector3f &normal, const vector2f &uv); //lmr static mesh
	//virtual void Reserve(unsigned int howmuch)

	//could make these private, but it is nice to be able to
	//add attributes separately...
	std::vector<vector3f> position;
	std::vector<vector3f> normal;
	std::vector<Color> diffuse;
	std::vector<vector2f> uv0;

private:
	int m_attribs;
};

}

#endif
