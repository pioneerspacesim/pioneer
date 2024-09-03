// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NODE_H
#define _NODE_H
/*
 * Generic node for the model scenegraph
 */
#include "RefCounted.h"
#include "graphics/Material.h"

#include <string>
#include <vector>

namespace Graphics {
	class Renderer;
}

namespace Serializer {
	class Reader;
	class Writer;
} // namespace Serializer

namespace SceneGraph {

	class BaseLoader;
	class NodeVisitor;
	class NodeCopyCache;
	class Model;
	class Group;

	//Node traversal mask - for other
	//purposes, use NodeFlags
	enum NodeMask {
		NODE_SOLID = 0x1,
		NODE_TRANSPARENT = 0x2,
		MASK_IGNORE = 0x4
	};

	//misc flags to identify features
	enum NodeFlags {
		NODE_TAG = 0x1,
		NODE_DECAL = 0x2
	};

	//Small structure used internally to pass rendering data
	struct RenderData {
		float linthrust[3]; // 1.0 to -1.0
		float angthrust[3]; // 1.0 to -1.0
		Color customColor;

		float boundingRadius; //updated by model and passed to submodels
		unsigned int nodemask;

		RenderData() :
			linthrust(),
			angthrust(),
			boundingRadius(0.f),
			nodemask(NODE_SOLID) //draw solids
		{
		}
	};

	//Collection of stuff nodes need for serialization -
	//makes maintaining function signatures easier
	struct NodeDatabase {
		Serializer::Writer *wr;
		Serializer::Reader *rd;
		//	Graphics::Renderer *renderer;
		Model *model;
		std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material>>> *materials;
		BaseLoader *loader;
	};

	class Node : public RefCounted {
	public:
		Node(Graphics::Renderer *r);
		Node(Graphics::Renderer *r, unsigned int nodemask);
		Node(const Node &, NodeCopyCache *);
		virtual Node *Clone(NodeCopyCache *) = 0; //implement clone to return shallow or deep copy
		virtual const char *GetTypeName() const { return "Node"; }
		virtual void Save(NodeDatabase &);

		virtual void Accept(NodeVisitor &v);
		virtual void Traverse(NodeVisitor &v);
		virtual void Render(const matrix4x4f &trans, const RenderData *rd) {}
		virtual void Render(const std::vector<matrix4x4f> &trans, const RenderData *rd) {}
		void DrawAxes();
		void SetName(const std::string &name) { m_name = name; }
		const std::string &GetName() const { return m_name; }

		void SetParent(Group *parent) { m_parent = parent; }
		Group *GetParent() { return m_parent; }
		const Group *GetParent() const { return m_parent; }

		virtual Node *FindNode(const std::string &);

		unsigned int GetNodeMask() const { return m_nodeMask; }
		void SetNodeMask(unsigned int m) { m_nodeMask = m; }

		unsigned int GetNodeFlags() const { return m_nodeFlags; }
		void SetNodeFlags(unsigned int m) { m_nodeFlags = m; }

		Graphics::Renderer *GetRenderer() const { return m_renderer; }

	protected:
		//can only to be deleted using DecRefCount
		virtual ~Node();
		std::string m_name;
		unsigned int m_nodeMask;
		unsigned int m_nodeFlags;
		Graphics::Renderer *m_renderer;
		Group *m_parent;
	};

} // namespace SceneGraph

#endif
