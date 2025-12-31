// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VertexArray.h"
#include "VertexBuffer.h"

#include "profiler/Profiler.h"

namespace Graphics {

	// Copy structs and methods for VertexArray::Populate()

	#pragma pack(push, 4)
	struct PosUVVert {
		vector3f pos;
		vector2f uv;
	};

	struct PosNormVert {
		vector3f pos;
		vector3f norm;
	};

	struct PosColVert {
		vector3f pos;
		Color4ub col;
	};

	struct PosVert {
		vector3f pos;
	};

	struct PosColUVVert {
		vector3f pos;
		Color4ub col;
		vector2f uv;
	};

	struct PosNormUVVert {
		vector3f pos;
		vector3f norm;
		vector2f uv;
	};

	struct PosNormColVert {
		vector3f pos;
		vector3f norm;
		Color4ub col;
	};
	#pragma pack(pop)

	static inline void CopyPosNorm(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosNormVert *vtxPtr = vb->Map<PosNormVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosNormVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].norm = va.normal[i];
		}
		vb->Unmap();
	}

	static inline void CopyPosUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosUVVert *vtxPtr = vb->Map<PosUVVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosUVVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].uv = va.uv0[i];
		}
		vb->Unmap();
	}

	static inline void CopyPosCol(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosColVert *vtxPtr = vb->Map<PosColVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosColVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].col = va.diffuse[i];
		}
		vb->Unmap();
	}

	static inline void CopyPos(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosVert *vtxPtr = vb->Map<PosVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
		}
		vb->Unmap();
	}

	static inline void CopyPosColUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosColUVVert *vtxPtr = vb->Map<PosColUVVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosColUVVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].col = va.diffuse[i];
			vtxPtr[i].uv = va.uv0[i];
		}
		vb->Unmap();
	}

	static inline void CopyPosNormUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosNormUVVert *vtxPtr = vb->Map<PosNormUVVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosNormUVVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].norm = va.normal[i];
			vtxPtr[i].uv = va.uv0[i];
		}
		vb->Unmap();
	}

	static inline void CopyPosNormCol(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
	{
		PosNormColVert *vtxPtr = vb->Map<PosNormColVert>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetStride() == sizeof(PosNormColVert));
		for (Uint32 i = 0; i < va.GetNumVerts(); i++) {
			vtxPtr[i].pos = va.position[i];
			vtxPtr[i].norm = va.normal[i];
			vtxPtr[i].col = va.diffuse[i];
		}
		vb->Unmap();
	}

	// ========================================================================

	VertexArray::VertexArray(AttributeSet attribs, int size)
	{
		PROFILE_SCOPED()
		m_attribs = attribs;

		if (size > 0)
			Reserve(size);
	}

	VertexArray::~VertexArray()
	{
	}

	void VertexArray::Reserve(uint32_t size)
	{
		if (m_attribs & ATTRIB_POSITION)
			position.reserve(size);
		if (m_attribs & ATTRIB_DIFFUSE)
			diffuse.reserve(size);
		if (m_attribs & ATTRIB_NORMAL)
			normal.reserve(size);
		if (m_attribs & ATTRIB_UV0)
			uv0.reserve(size);
		if (m_attribs & ATTRIB_TANGENT)
			tangent.reserve(size);
	}

	void VertexArray::Clear(uint32_t size)
	{
		position.clear();
		diffuse.clear();
		normal.clear();
		uv0.clear();
		tangent.clear();

		if (size > 0)
			Reserve(size);
	}

	bool VertexArray::Populate(VertexBuffer *buffer) const
	{
		PROFILE_SCOPED()
		assert(GetNumVerts() > 0);
		assert(GetNumVerts() <= buffer->GetCapacity());
		bool result = false;

		switch (m_attribs) {
		case Graphics::ATTRIB_POSITION:
			CopyPos(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE:
			CopyPosCol(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL:
			CopyPosNorm(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0:
			CopyPosUV0(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0:
			CopyPosColUV0(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0:
			CopyPosNormUV0(buffer, *this);
			result = true;
			break;
		case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_DIFFUSE:
			CopyPosNormCol(buffer, *this);
			result = true;
			break;
		}

		buffer->SetVertexCount(GetNumVerts());
		return result;
	}

	bool VertexArray::PopulateRange(const VertexFormatDesc &fmt, uint8_t *buffer, size_t size) const
	{
		size_t range = GetNumVerts() * fmt.bindings[0].stride;

		if (size < range)
			return false;

		// Get an enumerated list of the attributes in this set
		VertexAttrib semantics[MAX_ATTRIBS] = {};

		uint32_t numAttrs = fmt.GetNumAttribs();
		GetActiveAttribsInSet(m_attribs, semantics, numAttrs);

		// Complicated-ish loop to deal with 16+ possible combinations of vertex formats
		// (Position is effectively required, or it would be 32)
		for (size_t idx = 0; idx < GetNumVerts(); idx++) {
			for (uint32_t n = 0; n < numAttrs; n++) {
				// Calculate the location of this component inside the vertex being written
				uint8_t *data = buffer + idx * fmt.bindings[0].stride + fmt.attribs[n].offset;
				switch (semantics[n]) {
				case ATTRIB_POSITION:
					*reinterpret_cast<vector3f *>(data) = position[idx];
					break;
				case ATTRIB_NORMAL:
					*reinterpret_cast<vector3f *>(data) = normal[idx];
					break;
				case ATTRIB_DIFFUSE:
					*reinterpret_cast<Color4ub *>(data) = diffuse[idx];
					break;
				case ATTRIB_UV0:
					*reinterpret_cast<vector2f *>(data) = uv0[idx];
					break;
				case ATTRIB_TANGENT:
					*reinterpret_cast<vector3f *>(data) = tangent[idx];
					break;
				default:
					assert(false && "Unimplemented vertex attribute in VertexArray::PopulateRange!");
					break;
				}
			}
		}

		return true;
	}

	void VertexArray::Add(const vector3f &v)
	{
		position.emplace_back(v);
	}

	void VertexArray::Add(const vector3f &v, const Color &c)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
	}

	void VertexArray::Add(const vector3f &v, const Color &c, const vector3f &n)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
		normal.emplace_back(n);
	}

	void VertexArray::Add(const vector3f &v, const Color &c, const vector2f &uv)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector2f &uv)
	{
		position.emplace_back(v);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n, const vector2f &uv)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
		uv0.emplace_back(uv);
		tangent.emplace_back(tang);
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v)
	{
		position[idx] = v;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c)
	{
		position[idx] = v;
		diffuse[idx] = c;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c, const vector3f &n)
	{
		position[idx] = v;
		diffuse[idx] = c;
		normal[idx] = n;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c, const vector2f &uv)
	{
		position[idx] = v;
		diffuse[idx] = c;
		uv0[idx] = uv;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector2f &uv)
	{
		position[idx] = v;
		uv0[idx] = uv;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n)
	{
		position[idx] = v;
		normal[idx] = n;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv)
	{
		position[idx] = v;
		normal[idx] = n;
		uv0[idx] = uv;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang)
	{
		position[idx] = v;
		normal[idx] = n;
		uv0[idx] = uv;
		tangent[idx] = tang;
	}

} // namespace Graphics
