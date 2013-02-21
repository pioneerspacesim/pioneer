// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <map>

#ifndef SCENEGRAPH_NODECOPYCACHE_H
#define SCENEGRAPH_NODECOPYCACHE_H

namespace SceneGraph {

class Node;

class NodeCopyCache {
public:
	template <typename T> T *Copy(const T *origNode) {
		std::map<const Node*,Node*>::const_iterator i = m_cache.find(origNode);
		if (i != m_cache.end())
			return static_cast<T*>((*i).second);
		T *newNode = new T(*origNode, this);
		m_cache.insert(std::make_pair(origNode, newNode));
		return newNode;
	}

private:
	std::map<const Node*,Node*> m_cache;
};

}

#endif
