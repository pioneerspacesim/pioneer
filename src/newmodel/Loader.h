#ifndef _LOADER_H
#define _LOADER_H
/*
 * Load models using the model config files under data/newmodels/
 */
#include "libs.h"
#include "NModel.h"
#include "graphics/Material.h"

struct aiNode;

namespace Graphics { class Renderer; }

namespace Newmodel {

struct MaterialDefinition {
	MaterialDefinition() :
		name(""),
		tex_diff(""),
		tex_spec(""),
		tex_glow(""),
		diffuse(Color(1.f)),
		specular(Color(1.f)),
		ambient(Color(0.f)),
		emissive(Color(0.f)),
		shininess(200)
	{ }
	std::string name;
	std::string tex_diff;
	std::string tex_spec;
	std::string tex_glow;
	Color diffuse;
	Color specular;
	Color ambient;
	Color emissive;
	int shininess; //specular power, 0+
};

struct LodDefinition {
	LodDefinition(float size) : pixelSize(size) { }
	float pixelSize;
	std::vector<std::string> meshNames;
};

struct TagDefinition {
	TagDefinition(const std::string &tagname, const vector3f &pos) :
		name(tagname), position(pos) { }
	std::string name;
	vector3f position;
};
typedef std::vector<TagDefinition> TagList;

struct ModelDefinition {
	std::string name;
	std::vector<LodDefinition> lodDefs;
	std::vector<MaterialDefinition> matDefs;
	TagList tagDefs;
};

class Loader {
public:
	//renderer needed for texture loading...
	Loader(Graphics::Renderer *r);
	~Loader();
	//find & attempt to load a model, based on filename (without path or .model suffix)
	NModel *LoadModel(const std::string &name);
	NModel *LoadModel(const std::string &name, const std::string &basepath);

private:
	Graphics::Renderer *m_renderer;
	std::string m_curPath;
	Graphics::Texture *GetWhiteTexture() const;
	NModel *CreateModel(ModelDefinition &def);
	void FindTags(const aiNode *node, TagList &output); //locate tags from assimp structure
	//load one mesh file so it can be added to the model scenegraph. Materials should be created before this!
	Node *LoadMesh(const std::string &filename, const NModel *model, TagList &modelTags);
};

}
#endif
