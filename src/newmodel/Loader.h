#ifndef _LOADER_H
#define _LOADER_H
/*
 * Load models using the model config files under data/newmodels/
 */
#include "libs.h"
class lua_State;
namespace Graphics { class Renderer; }
namespace Newmodel {

class NModel;
class Node;

struct MaterialDefinition {
	std::string name;
	std::string diffuseTexture;
};

struct ModelDefinition {
	std::string name;
	std::vector<std::string> meshNames;
	std::vector<MaterialDefinition> matDefs;
};

class Loader {
public:
	//renderer needed for texture loading...
	Loader(Graphics::Renderer *r);
	~Loader();
	//find & attempt to load a model, based on filename (without path or .lua suffix)
	NModel *LoadModel(const std::string &name);
private:
	NModel *CreateModel(const ModelDefinition &def);
	//load one mesh file so it can be added to the model scenegraph. Materials should be created before this!
	Node *LoadMesh(const std::string &filename);
	std::string m_curPath;
private:
	lua_State *m_luaState;
};

}
#endif
