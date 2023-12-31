// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Tag.h"

#include "matrix4x4.h"

#include "scenegraph/BaseLoader.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/NodeCopyCache.h"
#include "scenegraph/NodeVisitor.h"
#include "scenegraph/Serializer.h"

namespace SceneGraph {

	Tag::Tag(Graphics::Renderer *r, const matrix4x4f &m) :
		MatrixTransform(r, m),
		m_globalTransform(matrix4x4f::Identity())
	{
	}

	Tag::Tag(const Tag &tag, NodeCopyCache *cache) :
		MatrixTransform(tag, cache),
		m_globalTransform(tag.m_globalTransform)
	{
	}

	Node *Tag::Clone(NodeCopyCache *cache)
	{
		return cache->Copy<Tag>(this);
	}

	void Tag::Accept(NodeVisitor &nv)
	{
		nv.ApplyTag(*this);
	}

	void Tag::Save(NodeDatabase &db)
	{
		MatrixTransform::Save(db);
	}

	Tag *Tag::Load(NodeDatabase &db)
	{
		// Read matrix data from MatrixTransform
		matrix4x4f matrix;
		for (Uint32 i = 0; i < 16; i++)
			matrix[i] = db.rd->Float();

		Tag *mt = new Tag(db.loader->GetRenderer(), matrix);
		return mt;
	}

	void Tag::UpdateGlobalTransform()
	{
		m_globalTransform = CalcGlobalTransform();
	}

} // namespace SceneGraph
