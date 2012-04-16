#ifndef _LOADER_H
#define _LOADER_H
/*
 * Load models using the model config files under data/newmodels/
 */
#include "libs.h"
#include "NModel.h"
#include "graphics/Material.h"

namespace Graphics { class Renderer; }

namespace Newmodel {

struct MaterialDefinition {
	MaterialDefinition() :
		name(""),
		tex_diff(""),
		tex_spec(""),
		diffuse(Color(1.f)),
		specular(Color(1.f)),
		ambient(Color(0.f)),
		emissive(Color(0.f)),
		shininess(0)
	{ }
	std::string name;
	std::string tex_diff;
	std::string tex_spec;
	Color diffuse;
	Color specular;
	Color ambient;
	Color emissive;
	int shininess; //specular power, 0+
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
	//find & attempt to load a model, based on filename (without path or .model suffix)
	NModel *LoadModel(const std::string &name);

private:
	Graphics::Renderer *m_renderer;
	Graphics::Texture *GetWhiteTexture() const;
	NModel *CreateModel(const ModelDefinition &def);
	//load one mesh file so it can be added to the model scenegraph. Materials should be created before this!
	Node *LoadMesh(const std::string &filename, const NModel *model);
	std::string m_curPath;
};

}
#endif
