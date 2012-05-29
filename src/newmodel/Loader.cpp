#include "Loader.h"
#include "FileSystem.h"

#include "MatrixTransform.h"
#include "Newmodel.h"
#include "StaticGeometry.h"
#include "LOD.h"
#include "graphics/Renderer.h"
#include "graphics/Surface.h"
#include "graphics/TextureBuilder.h"
#include <sstream>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

//debugging
#include <iostream>

namespace Newmodel {

#if 1 //begin parser
bool LodSortPredicate(const LodDefinition &a, const LodDefinition &b)
{
	return a.pixelSize < b.pixelSize;
}

class Parser
{
public:
	Parser(const std::string &filename, const std::string &path) :
		m_isMaterial(false),
		m_curMat(0),
		m_model(0),
		m_path(path)
	{
		m_file.open(filename.c_str(), std::ifstream::in);
		if (!m_file) throw std::string("Could not open " + filename);
	}

	void parse(ModelDefinition *m) {
		m_model = m;
		char line[1024];
		int lineno = 0;
		while (m_file.good()) {
			lineno++;
			m_file.getline(line, 1023);
			try {
				if (!parseLine(std::string(line)))
					throw std::string("Mystery fail");
			} catch (const std::string &s) {
				std::stringstream ss;
				ss << "Error parsing line " << lineno << ":" << std::endl;
				ss << line << std::endl;
				ss << s;
				throw ss.str();
			}
		}
		//sort lods by feature size
		std::sort(m->lodDefs.begin(), m->lodDefs.end(), LodSortPredicate);
	}
	~Parser() {
		m_file.close();
	}

private:
	bool m_isMaterial;
	MaterialDefinition *m_curMat;
	ModelDefinition *m_model;
	std::ifstream m_file;
	std::string m_path;

	bool isComment(const std::string &s) {
		assert(!s.empty());
		return (s[0] == '#');
	}

	//check if string matches completely
	bool match(const std::string &s, const std::string &what) {
		return (s.compare(what) == 0);
	}

	bool checkTexture(std::stringstream &ss, std::string &out) {
		if (ss >> out == 0) throw std::string("Expected file name, got nothing");
		if (isComment(out)) throw std::string("Expected file name, got comment");
		//add newmodels/some_model/ to path
		out = FileSystem::JoinPathBelow(m_path, out);
		return true;
	}

	inline bool checkMesh(std::stringstream &ss, std::string &out) {
		return checkTexture(ss, out);
	}

	inline bool checkMaterialName(std::stringstream &ss, std::string &out) {
		if (ss >> out == 0) throw std::string("Expected material name, got nothing");
		if (isComment(out)) throw std::string("Expected material name, got comment");
		return true;
	}

	bool checkColor(std::stringstream &ss, Color &color) {
		float r, g, b;
		ss >> r >> g >> b;
		color.r = Clamp(r, 0.f, 1.f);
		color.g = Clamp(g, 0.f, 1.f);
		color.b = Clamp(b, 0.f, 1.f);
		color.a = 1.f; //yeah, we don't support alpha
		return true;
	}

	bool parseLine(const std::string &line) {
		using std::stringstream;
		using std::string;
		stringstream ss(stringstream::in | stringstream::out);
		ss.str(line);
		if (ss.fail()) return false;
		string token;
		if ((ss >> token) != 0) {
			//line contains something
			if (isComment(token))
				return true; //skip comments
			if (match(token, "material")) {
				//beginning of a new material definition,
				//expect a name and then parameters on following lines
				m_isMaterial = true;
				string matname;
				checkMaterialName(ss, matname);
				m_model->matDefs.push_back(MaterialDefinition());
				m_curMat = &m_model->matDefs.back();
				m_curMat->name = matname;
				return true;
			} else if(match(token, "lod")) {
				m_isMaterial = false;
				m_curMat = 0;
				float featuresize;
				if (ss >> featuresize == 0)
					throw std::string("Detail level must specify a pixel size");
				if (is_zero_general(featuresize))
					throw std::string("Detail level pixel size must be greater than 0");
				m_model->lodDefs.push_back(LodDefinition(featuresize));
				return true;
			} else if(match(token, "mesh")) {
				//mesh definitionss only contain a filename
				m_isMaterial = false;
				m_curMat = 0;
				string meshname;
				checkMesh(ss, meshname);
				//model might not have specified lods at all.
				if (m_model->lodDefs.empty()) {
					m_model->lodDefs.push_back(LodDefinition(100.f));
				}
				m_model->lodDefs.back().meshNames.push_back(meshname);
				return true;
			} else {
				if (m_isMaterial) {
					//material definition in progress, check known parameters
					if (match(token, "tex_diff"))
						return checkTexture(ss, m_curMat->tex_diff);
					else if (match(token, "tex_spec"))
						return checkTexture(ss, m_curMat->tex_spec);
					else if (match(token, "tex_glow"))
						return checkTexture(ss, m_curMat->tex_glow);
					else if (match(token, "diffuse"))
						return checkColor(ss, m_curMat->diffuse);
					else if (match(token, "specular"))
						return checkColor(ss, m_curMat->specular);
					else if (match(token, "ambient"))
						return checkColor(ss, m_curMat->ambient);
					else if (match(token, "emissive"))
						return checkColor(ss, m_curMat->emissive);
					else if (match(token, "shininess")) {
						int shininess;
						ss >> shininess;
						m_curMat->shininess = std::max(shininess, 0);
						return true;
					}
					else if (match(token, "use_patterns")) {
						m_curMat->use_pattern = true;
						return true;
					}
					else //unknown instruction
						return false;
				}
				return false;
			}
		} else {
			//empty line, skip
			return true;
		}
	}
};
#endif //end parser

Loader::Loader(Graphics::Renderer *r) :
	m_renderer(r)
{
}

Loader::~Loader()
{
}

NModel *Loader::LoadModel(const std::string &filename)
{
	NModel *m = LoadModel(filename, "newmodels");
	if (m) return m;
	throw LoadingError();
}

NModel *Loader::LoadModel(const std::string &filename, const std::string &basepath)
{
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetAbsolutePath();

		if (info.IsDir()) {
			NModel *m = LoadModel(filename, info.GetPath());
			if (m) return m;
		}
		//check it's the expected type
		else if (info.IsFile() && (fpath.substr(fpath.find_last_of(".")+1) == "model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();
			//XXX hmm
			m_curPath = info.GetDir();
			if (filename == name.substr(0, name.length()-6)) {
				ModelDefinition modelDefinition;
				try {
					//XXX use filesystem and load the file as a string
					Parser p(fpath, m_curPath);
					p.parse(&modelDefinition);
				} catch (const std::string &str) {
					std::cerr << str << std::endl;
					throw LoadingError();
				}
				modelDefinition.name = name.substr(0, name.length()-6);
				return CreateModel(modelDefinition);
			}
		}
		
	}
	return 0;
}

Graphics::Texture *Loader::GetWhiteTexture() const
{
	return Graphics::TextureBuilder::Model("textures/white.png").GetOrCreateTexture(m_renderer, "model");
}

NModel *Loader::CreateModel(ModelDefinition &def)
{
	using Graphics::Material;
	if (def.matDefs.empty()) return 0;
	if (def.lodDefs.empty()) return 0;

	NModel *model = new NModel(def.name);
	bool patternsUsed = false;

	//create materials from definitions
	for(std::vector<MaterialDefinition>::const_iterator it = def.matDefs.begin();
		it != def.matDefs.end(); ++it)
	{
		assert(!(*it).name.empty());
		const std::string &diffTex = (*it).tex_diff;
		const std::string &specTex = (*it).tex_spec;
		const std::string &glowTex = (*it).tex_glow;

		Graphics::MaterialDescriptor matDesc;
		if ((*it).use_pattern) {
			patternsUsed = true;
			matDesc.usePatterns = true;
		}
		matDesc.glowMap = !glowTex.empty();
		matDesc.specularMap = !specTex.empty();

		RefCountedPtr<Material> mat(m_renderer->CreateMaterial(matDesc));
		mat->diffuse = (*it).diffuse;
		mat->specular = (*it).specular;
		mat->emissive = (*it).emissive;
		mat->shininess = (*it).shininess;

		//XXX white texture is sort of a workaround when all textures are not specified
		if (!diffTex.empty())
			mat->texture0 = Graphics::TextureBuilder::Model(diffTex).GetOrCreateTexture(m_renderer, "model");
		else
			mat->texture0 = GetWhiteTexture();
		if (!specTex.empty())
			mat->texture1 = Graphics::TextureBuilder::Model(specTex).GetOrCreateTexture(m_renderer, "model");
		if (!glowTex.empty())
			mat->texture2 = Graphics::TextureBuilder::Model(glowTex).GetOrCreateTexture(m_renderer, "model");

		model->m_materials.push_back(std::make_pair<std::string, RefCountedPtr<Material> >((*it).name, mat));
	}
	//printf("Loaded %d materials\n", int(model->m_materials.size()));

	//load meshes
	std::map<std::string, Node*> meshCache;
	LOD *lodNode = 0;
	if (def.lodDefs.size() > 1) { //don't bother with a lod node if only one level
		lodNode = new LOD();
		model->GetRoot()->AddChild(lodNode);
	}
	for(std::vector<LodDefinition>::const_iterator lod = def.lodDefs.begin();
		lod != def.lodDefs.end(); ++lod)
	{
		//does a detail level have multiple meshes? If so, we need a Group.
		Group *group = 0;
		if (lodNode && (*lod).meshNames.size() > 1) {
			group = new Group();
			lodNode->AddLevel((*lod).pixelSize, group);
		}
		for(std::vector<std::string>::const_iterator it = (*lod).meshNames.begin();
			it != (*lod).meshNames.end(); ++it)
		{
			try {
				//multiple lods might use the same mesh
				Node *mesh = 0;
				std::map<std::string, Node*>::iterator cacheIt = meshCache.find((*it));
				if (cacheIt != meshCache.end())
					mesh = (*cacheIt).second;
				else {
					mesh = LoadMesh(*(it), model, def.tagDefs);
					meshCache[*(it)] = mesh;
				}

				if (group)
					group->AddChild(mesh);
				else if(lodNode)
					lodNode->AddLevel((*lod).pixelSize, mesh);
				else
					model->GetRoot()->AddChild(mesh);
			} catch (LoadingError &) {
				delete model;
				throw;
			} catch (const std::string &s) {
				delete model;
				std::cout << s << std::endl;
				throw LoadingError();
			}
		}
	}

	Animation *anim = 0;

	//add some dummy tag points
	for(TagList::const_iterator it = def.tagDefs.begin();
		it != def.tagDefs.end();
		++it)
	{
		if (!anim) anim = new Animation("wiggle");
		const vector3f &pos = (*it).position;
		MatrixTransform *tagTrans = new MatrixTransform(matrix4x4f::Translation(pos.x, pos.y, pos.z));
		//add a test animation for the tag (which is silly)
		anim->channels.push_back(AnimationChannel(tagTrans));
		AnimationChannel &chan = anim->channels.back();
		chan.rotationKeys.push_back(RotationKey(0.0, Quaternionf(1.f, 0.f, 0.f, 0.f)));
		chan.rotationKeys.push_back(RotationKey(50.0, Quaternionf(1.5707f, vector3f(1.f, 0.f, 0.f))));
		chan.rotationKeys.push_back(RotationKey(100.0, Quaternionf(1.f, 0.f, 0.f, 0.f)));
		chan.positionKeys.push_back(PositionKey(0.0, pos));
		chan.positionKeys.push_back(PositionKey(50.0, pos + vector3f(0.f, 3.f, 0.f)));
		chan.positionKeys.push_back(PositionKey(100.0, pos));
		model->AddTag((*it).name, tagTrans);
	}

	if (anim) model->m_animations.push_back(anim);

	//find usable pattern textures from the model directory
	if (patternsUsed) {
		FindPatterns(model->m_patterns);
		//set up some noticeable default colors
		std::vector<Color4ub> colors;
		colors.push_back(Color4ub(255, 0, 0, 0));
		colors.push_back(Color4ub(0, 255, 0, 0));
		colors.push_back(Color4ub(0, 0, 255, 0));
		model->SetColors(m_renderer, colors);
		model->SetPattern(0);
	}	
	return model;
}

void Loader::FindTags(const aiNode *node, TagList &output)
{
	const std::string nodename(node->mName.C_Str());
	static const std::string tagIdentifier("tag_");
	if (nodename.compare(0, tagIdentifier.length(), tagIdentifier) == 0) {
		aiVector3D position;
		aiQuaternion rotation;
		node->mTransformation.DecomposeNoScaling(rotation, position);
		output.push_back(TagDefinition(nodename, vector3f(position.x, position.y, position.z)));
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		aiNode *child = node->mChildren[i];
		FindTags(child, output);
	}
}

void Loader::FindPatterns(PatternContainer &output)
{
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, m_curPath); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		if (info.IsFile()) {
			const std::string &name = info.GetName();
			if (name.substr(name.find_last_of(".")+1) == "png") { //correct type?
				if(name.compare(0, 7, "pattern") == 0) { //acceptable name?
					//load as a pattern
					const std::string &patternPath = FileSystem::JoinPathBelow(m_curPath, name);
					Graphics::Texture *pat = Graphics::TextureBuilder::Model(patternPath).CreateTexture(m_renderer);
					output.push_back(Pattern(name, pat));
				}
			}
			//std::cout << files.Current().GetName() << std::endl;
		}
	}
	//std::cout << m_curPath << std::endl;
	
}

Node *Loader::LoadMesh(const std::string &filename, NModel *model, TagList &modelTags)
{
	Assimp::Importer importer;
	//assimp needs the data dir too...
	//XXX check user dir first
	//XXX x2 the greater goal is not to use ReadFile but the other assimp data read functions + FileSystem. See assimp docs.
	const aiScene *scene = importer.ReadFile(
		FileSystem::JoinPath(FileSystem::GetDataDir(), filename),
		aiProcess_Triangulate   |
		aiProcess_SortByPType   | //ignore point, line primitive types (collada dummy nodes seem to be fine)
		aiProcess_GenUVCoords   | //only if they don't exist
		aiProcess_FlipUVs		|
		aiProcess_GenSmoothNormals); //only if normals not specified

	if(!scene)
		throw std::string("Couldn't load " + filename);

	if(scene->mNumMeshes == 0)
		throw std::string(filename + " has no geometry. How odd!");

	//turn all scene aiMeshes into Surfaces
	//Index matches assimp index.
	std::vector<Graphics::Surface*> surfaces;
	ConvertAiMeshesToSurfaces(surfaces, scene, model);

	Node *node = 0; //XXX don't return a node, pass in the root (of the current LOD, not model root)

	if (!scene->HasAnimations()) {
		//XXX putting everything in one static mesh
		//XXX the plan: if scene has animation, go through the assimp node structure and
		//create the appropriate nodes (staticgeometry parented to matrixtransforms for animated nodes)
		StaticGeometry *geom = new StaticGeometry();
		Graphics::StaticMesh *smesh = geom->GetMesh();
		node = geom;

		//the entire mesh might be translated, the usual case is 90 deg X rotation
		MatrixTransform *trans = 0;
		if (!scene->mRootNode->mTransformation.IsIdentity()) {
			matrix4x4f m = ConvertMatrix(scene->mRootNode->mTransformation);
			trans = new MatrixTransform(m);
			trans->AddChild(geom);
			node = trans;
		}

		//update bounding box
		for (unsigned int i=0; i<surfaces.size(); i++) {
			Graphics::Surface *surf = surfaces[i];
			Graphics::VertexArray *vts = surf->GetVertices();
			for (unsigned int j=0; j<vts->position.size(); j++) {
				const vector3f &vtx = vts->position[j];
				geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
			}
			smesh->AddSurface(surf);
		}
	} else {
		Group *group = new Group;
		ConvertNodes(scene->mRootNode, group, surfaces);
		node = group;
	}

	m_root = node;

	ConvertAnimations(scene, model);

	//try to figure out tag points, in case we happen to use an
	//advanced file format (collada)
	FindTags(scene->mRootNode, modelTags);

	return node;
}

void Loader::ConvertAiMeshesToSurfaces(std::vector<Graphics::Surface*> &surfaces, const aiScene *scene, const NModel *model)
{
	//XXX sigh, workaround for obj loader
	int matIdxOffs = 0;
	if (scene->mNumMaterials > scene->mNumMeshes)
		matIdxOffs = 1;

	//turn meshes into surfaces
	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		assert(mesh->HasNormals());
		if (!mesh->HasTextureCoords(0))
			throw std::string("Mesh has no uv coordinates");

		//Material names are not consistent throughout formats...
		//try to figure out a material
		//try name first, if that fails use index
		RefCountedPtr<Graphics::Material> mat;
		const aiMaterial *amat = scene->mMaterials[mesh->mMaterialIndex];
		aiString s;
		if(AI_SUCCESS == amat->Get(AI_MATKEY_NAME,s)) {
			//std::cout << "Looking for " << std::string(s.data,s.length) << std::endl;
			mat = model->GetMaterialByName(std::string(s.data, s.length));
		}

		if (!mat.Valid()) {
			mat = model->GetMaterialByIndex(mesh->mMaterialIndex - matIdxOffs);
		}

		assert(mat.Valid());

		Graphics::VertexArray *vts =
			new Graphics::VertexArray(
				Graphics::ATTRIB_POSITION |
				Graphics::ATTRIB_NORMAL |
				Graphics::ATTRIB_UV0);

		Graphics::Surface *surface = new Graphics::Surface(Graphics::TRIANGLES, vts, mat);
		std::vector<unsigned short> &indices = surface->GetIndices();

		//copy indices first
		//note: index offsets are not adjusted, StaticMesh should do that for us
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			const aiFace *face = &mesh->mFaces[f];
			for (unsigned int j = 0; j < face->mNumIndices; j++) {
				indices.push_back(face->mIndices[j]);
			}
		}

		//then vertices, making gross assumptions of the format
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &vtx = mesh->mVertices[v];
			const aiVector3D &norm = mesh->mNormals[v];
			const aiVector3D &uv0 = mesh->mTextureCoords[0][v];
			vts->Add(vector3f(vtx.x, vtx.y, vtx.z),
				vector3f(norm.x, norm.y, norm.z),
				vector2f(uv0.x, uv0.y));
		}

		surfaces.push_back(surface);
	}
}

void Loader::ConvertAnimations(const aiScene* scene, NModel *model)
{
	std::vector<Animation*> &animations = model->m_animations;

	for (unsigned int i=0; i<scene->mNumAnimations; i++) {
		const aiAnimation* aianim = scene->mAnimations[i];
		const std::string animname(aianim->mName.C_Str());
		Animation *animation = new Animation(animname.empty() ? "wiggle" : animname);
		for (unsigned int j=0; j<aianim->mNumChannels; j++) {
			aiNodeAnim *aichan = aianim->mChannels[j];
			const std::string channame(aichan->mNodeName.C_Str());
			MatrixTransform *trans = dynamic_cast<MatrixTransform*>(m_root->FindNode(channame));
			assert(trans);
			animation->channels.push_back(AnimationChannel(trans));
			AnimationChannel &chan = animation->channels.back();

			for(unsigned int k=0; k<aichan->mNumPositionKeys; k++) {
				const aiVectorKey &aikey = aichan->mPositionKeys[k];
				const aiVector3D &aipos = aikey.mValue;
				chan.positionKeys.push_back(PositionKey(aikey.mTime, vector3f(aipos.x, aipos.y, aipos.z)));
			}

			if (aichan->mNumRotationKeys < 2) continue;
			for(unsigned int k=0; k<aichan->mNumRotationKeys; k++) {
				const aiQuatKey &aikey = aichan->mRotationKeys[k];
				const aiQuaternion &airot = aikey.mValue;
				chan.rotationKeys.push_back(RotationKey(aikey.mTime, Quaternionf(airot.x, airot.y, airot.z, airot.w)));
			}
		}
		animations.push_back(animation);
	}
}

matrix4x4f Loader::ConvertMatrix(const aiMatrix4x4& trans) const
{
	matrix4x4f m;
	m[0] = trans.a1;
	m[1] = trans.b1;
	m[2] = trans.c1;
	m[3] = trans.d1;

	m[4] = trans.a2;
	m[5] = trans.b2;
	m[6] = trans.c2;
	m[7] = trans.d2;

	m[8] = trans.a3;
	m[9] = trans.b3;
	m[10] = trans.c3;
	m[11] = trans.d3;

	m[12] = trans.a4;
	m[13] = trans.b4;
	m[14] = trans.c4;
	m[15] = trans.d4;
	return m;
}

void Loader::ConvertNodes(aiNode *node, Group *_parent, std::vector<Graphics::Surface*>& surfaces)
{
	//XXX could ignore nodes without children and meshes, or could check
	//if they are tags (lights, cameras are out of scope at the moment)
	Group *parent = _parent;
	const std::string nodename(node->mName.C_Str());
	const aiMatrix4x4& trans = node->mTransformation;
	matrix4x4f m = ConvertMatrix(trans);

	//if the transform is identity and the node is not animated,
	//could just add a group
	parent = new MatrixTransform(m);
	_parent->AddChild(parent);
	parent->SetName(nodename);

	if (node->mNumMeshes > 0) {
		//is this node animated? add a transform
		//does this node have children? Add a group
		StaticGeometry *geom = new StaticGeometry();
		Graphics::StaticMesh *smesh = geom->GetMesh();

		for(unsigned int i=0; i<node->mNumMeshes; i++) {
			Graphics::Surface *surf = surfaces[node->mMeshes[i]];
			//update bounding box
			Graphics::VertexArray *vts = surf->GetVertices();
			for (unsigned int j=0; j<vts->position.size(); j++) {
				const vector3f &vtx = m * vts->position[j];
				geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
			}
			smesh->AddSurface(surf);
		}

		parent->AddChild(geom);
	}

	for(unsigned int i=0; i<node->mNumChildren; i++) {
		aiNode *child = node->mChildren[i];
		ConvertNodes(child, parent, surfaces);
	}
}

}
