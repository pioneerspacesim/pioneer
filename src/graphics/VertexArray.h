// Copyright Â© 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _VERTEXARRAY_H
#define _VERTEXARRAY_H

#include "Types.h"
#include "Color.h"
#include "vector3.h"

#include <vector>

namespace Graphics {

	struct VertexFormatDesc;
	class VertexBuffer;

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
		VertexArray(AttributeSet attribs, int size = 0);
		~VertexArray();

		//check presence of an attribute
		__inline bool HasAttrib(const VertexAttrib v) const { return (m_attribs & v) != 0; }
		__inline Uint32 GetNumVerts() const { return static_cast<Uint32>(position.size()); }
		__inline AttributeSet GetAttributeSet() const { return m_attribs; }

		__inline bool IsEmpty() const { return position.empty(); }

		//removes vertices, does not deallocate space
		// if reserveSize != 0, ensures the array has enough space for at least that many elements
		void Clear(uint32_t reserveSize = 0);

		// Copy the vertices from this VertexArray into the given VertexBuffer.
		// The VertexBuffer must have a compatible vertex format (i.e. stride).
		// NOTE: only supports these formats:
		//  - POSITION | [DIFFUSE/NORMAL/UV0]
		//  - POSITION | DIFFUSE | [UV0]
		//  - POSITION | NORMAL  | [UV0/DIFFUSE]
		bool Populate(VertexBuffer *buffer) const;

		// Copy the vertices from this VertexArray into the given buffer.
		// Supports any combination of attributes, but is slightly slower than Populate().
		// Returns false if the passed buffer cannot hold this VertexArray.
		// Note that the passed fmt must have the same order of attributes as one created from VertexFormatDesc::FromAttribSet
		bool PopulateRange(const VertexFormatDesc &fmt, uint8_t *buffer, size_t size) const;

		// don't mix these
		void Add(const vector3f &v);
		void Add(const vector3f &v, const Color &c);
		void Add(const vector3f &v, const Color &c, const vector3f &n);
		void Add(const vector3f &v, const Color &c, const vector2f &uv);
		void Add(const vector3f &v, const vector2f &uv);
		void Add(const vector3f &v, const vector3f &n);
		void Add(const vector3f &v, const vector3f &n, const vector2f &uv);
		void Add(const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang);
		//virtual void Reserve(unsigned int howmuch)

		// don't mix these
		void Set(const Uint32 idx, const vector3f &v);
		void Set(const Uint32 idx, const vector3f &v, const Color &c);
		void Set(const Uint32 idx, const vector3f &v, const Color &c, const vector3f &normal);
		void Set(const Uint32 idx, const vector3f &v, const Color &c, const vector2f &uv);
		void Set(const Uint32 idx, const vector3f &v, const vector2f &uv);
		void Set(const Uint32 idx, const vector3f &v, const vector3f &n);
		void Set(const Uint32 idx, const vector3f &v, const vector3f &normal, const vector2f &uv);
		void Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang);

		//could make these private, but it is nice to be able to
		//add attributes separately...
		std::vector<vector3f> position;
		std::vector<vector3f> normal;
		std::vector<Color> diffuse;
		std::vector<vector2f> uv0;
		std::vector<vector3f> tangent;

	private:
		void Reserve(uint32_t size);
		AttributeSet m_attribs;
	};

} // namespace Graphics

#endif
