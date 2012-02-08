#ifndef _VERTEXARRAY_H
#define _VERTEXARRAY_H

#include "libs.h"

//allowed minimum of GL_MAX_VERTEX_ATTRIBS is 8 on ES2
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

// this is a generic collection of vertex attributes. Renderers do
// whatever they need to do with regards to the attribute set.
// Presence of an attribute is checked using vector size, so users are trusted
// to provide matching number of attributes
struct VertexArray {
	//specify attributes to be used, additionally reserve space for vertices
	VertexArray(AttributeSet attribs, int size=0);
	~VertexArray();

	//check presence of an attribute
	virtual bool HasAttrib(VertexAttrib v);
	virtual unsigned int GetNumVerts() const;
	virtual AttributeSet GetAttributeSet() const { return m_attribs; }

	virtual void Clear();
	//no, I don't really like this
	virtual void Add(const vector3f &v);
	virtual void Add(const vector3f &v, const Color &c);
	virtual void Add(const vector3f &v, const Color &c, const vector3f &normal);
	virtual void Add(const vector3f &v, const Color &c, const vector2f &uv);
	virtual void Add(const vector3f &v, const vector2f &uv);
	virtual void Add(const vector3f &v, const vector3f &normal, const vector2f &uv); //lmr static mesh
	//virtual void Reserve(unsigned int howmuch)

	//make these private after all?
	std::vector<vector3f> position;
	std::vector<vector3f> normal;
	std::vector<Color> diffuse;
	std::vector<vector2f> uv0;

private:
	int m_attribs;
};

#endif
