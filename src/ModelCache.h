// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELCACHE_H
#define _MODELCACHE_H
/*
 * This class is a quick thoughtless hack
 * Also it only deals in New Models
 */

#include <map>
#include <stdexcept>

namespace Graphics {
	class Renderer;
}
namespace SceneGraph {
	class Model;
}

class ModelCache {
public:
	struct ModelNotFoundException : public std::runtime_error {
		ModelNotFoundException() :
			std::runtime_error("Could not find model") {}
	};
	ModelCache(Graphics::Renderer *);
	~ModelCache();
	SceneGraph::Model *FindModel(const std::string &);
	void Flush();

private:
	typedef std::map<std::string, SceneGraph::Model *> ModelMap;
	ModelMap m_models;
	Graphics::Renderer *m_renderer;
};

#endif
