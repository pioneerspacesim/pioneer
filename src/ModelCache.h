#ifndef _MODELCACHE_H
#define _MODELCACHE_H
/*
 * This class is a quick thoughtless hack
 * Also it only deals in New Models
 */
#include "libs.h"
#include <stdexcept>

namespace Graphics {
	class Renderer;
}
namespace SceneGraph {
	class NModel;
}

class ModelCache {
public:
	struct ModelNotFoundException : public std::runtime_error {
		ModelNotFoundException() : std::runtime_error("Could not find model") { }
	};
	ModelCache(Graphics::Renderer*);
	~ModelCache();
	SceneGraph::NModel *FindModel(const std::string&);
	void Flush();

private:
	typedef std::map<std::string, SceneGraph::NModel*> ModelMap;
	ModelMap m_models;
	Graphics::Renderer *m_renderer;
};

#endif