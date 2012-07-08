#ifndef _MODELCACHE_H
#define _MODELCACHE_H
/*
 * This class is a quick thoughtless hack
 * Also it only deals in New Models
 */
#include "libs.h"

namespace Graphics {
	class Renderer;
}
namespace Newmodel {
	class NModel;
}

class ModelCache {
public:
	ModelCache(Graphics::Renderer*);
	~ModelCache();
	Newmodel::NModel *FindModel(const std::string&);
	void Flush();

private:
	typedef std::map<std::string, Newmodel::NModel*> ModelMap;
	ModelMap m_models;
	Graphics::Renderer *m_renderer;
};

#endif