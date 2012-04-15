#include "Loader.h"
#include "FileSystem.h"
#include "Newmodel.h"
#include "StaticGeometry.h"
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
class Parser
{
public:
	Parser(const std::string &filename) :
		m_isMaterial(false),
		m_curMat(0),
		m_model(0)
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
	}
	~Parser() {
		m_file.close();
	}

private:
	bool m_isMaterial;
	MaterialDefinition *m_curMat;
	ModelDefinition *m_model;
	std::ifstream m_file;

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
		return true;
	}

	inline bool checkMesh(std::stringstream &ss, std::string &out) {
		return checkTexture(ss, out);
	}

	inline bool checkMaterialName(std::stringstream &ss, std::string &out) {
		return checkTexture(ss, out);
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
			} else if(match(token, "mesh")) {
				//mesh definitionss only contain a filename
				m_isMaterial = false;
				m_curMat = 0;
				string meshname;
				checkMesh(ss, meshname);
				m_model->meshNames.push_back(meshname);
				return true;
			} else {
				if (m_isMaterial) {
					//material definition in progress, check known parameters
					if (match(token, "tex_diff"))
						return checkTexture(ss, m_curMat->tex_diff);
					else if (match(token, "tex_spec"))
						return checkTexture(ss, m_curMat->tex_spec);
					else if (match(token, "diffuse"))
						return checkColor(ss, m_curMat->diffuse);
					else if (match(token, "specular"))
						return checkColor(ss, m_curMat->specular);
					else if (match(token, "ambient"))
						return checkColor(ss, m_curMat->ambient);
					else if (match(token, "emissive"))
						return checkColor(ss, m_curMat->emissive);
					else if (match(token, "power")) {
						int power;
						ss >> power;
						m_curMat->power = Clamp(power, 0, 128);
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
	//XXX not recursive
	static const std::string basepath("newmodels");
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirectories); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetAbsolutePath();
		//check it's the expected type
		if (info.IsFile() && (fpath.substr(fpath.find_last_of(".")+1) == "model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();
			if (filename == name.substr(0, name.length()-6)) {
				ModelDefinition modelDefinition;
				try {
					//XXX use filesystem and load the file as a string
					Parser p(fpath);
					p.parse(&modelDefinition);
				} catch (const std::string &str) {
					std::cerr << str << std::endl;
					throw LoadingError();
				}
				modelDefinition.name = name.substr(0, name.length()-6);
				//XXX hmm
				m_curPath = info.GetDir();
				return CreateModel(modelDefinition);
			}
		}
		
	}
	throw LoadingError();
}

Graphics::Texture *Loader::GetWhiteTexture() const
{
	return Graphics::TextureBuilder::Model("textures/white.png").GetOrCreateTexture(m_renderer, "model");
}

NModel *Loader::CreateModel(const ModelDefinition &def)
{
	using Graphics::Material;
	if (def.matDefs.empty()) return 0;
	if (def.meshNames.empty()) return 0;

	NModel *model = new NModel(def.name);

	//create materials from definitions
	for(std::vector<MaterialDefinition>::const_iterator it = def.matDefs.begin();
		it != def.matDefs.end(); ++it)
	{
		assert(!(*it).name.empty());
		//XXX fix pathnames beforehand
		const std::string &diffTex = FileSystem::JoinPathBelow(m_curPath, (*it).tex_diff);
		const std::string &specTex = FileSystem::JoinPathBelow(m_curPath, (*it).tex_spec);
		RefCountedPtr<Material> mat(m_renderer->CreateMaterial());
		mat->diffuse = (*it).diffuse;
		mat->specular = (*it).specular;
		//XXX sort of a workaround when all textures are not specified
		if (!diffTex.empty())
			mat->texture0 = Graphics::TextureBuilder::Model(diffTex).GetOrCreateTexture(m_renderer, "model");
		else
			mat->texture0 = GetWhiteTexture();
		if (!specTex.empty())
			mat->texture1 = Graphics::TextureBuilder::Model(specTex).GetOrCreateTexture(m_renderer, "model");
		else
			mat->texture1 = GetWhiteTexture();
		model->m_materials.push_back(std::make_pair<std::string, RefCountedPtr<Material> >((*it).name, mat));
	}
	//printf("Loaded %d materials\n", int(model->m_materials.size()));

	//load meshes
	for(std::vector<std::string>::const_iterator it = def.meshNames.begin();
		it != def.meshNames.end(); ++it)
	{
		try {
			Node *mesh = LoadMesh(FileSystem::JoinPathBelow(m_curPath, *(it)), model);
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
	return model;
}

Node *Loader::LoadMesh(const std::string &filename, const NModel *model)
{
	Assimp::Importer importer;
	//assimp needs the data dir too...
	//XXX check user dir first
	//XXX x2 the greater goal is not to use ReadFile but the other assimp data read functions + FileSystem. See assimp docs.
	const aiScene *scene = importer.ReadFile(
		FileSystem::JoinPath(FileSystem::GetDataDir(), filename),
		aiProcess_OptimizeGraph |
		aiProcess_Triangulate   |
		aiProcess_SortByPType   |
		aiProcess_GenUVCoords   |
		aiProcess_GenSmoothNormals);

	if(!scene)
		throw std::string("Couldn't load " + filename);

	StaticGeometry *geom = new StaticGeometry();

	Graphics::StaticMesh *smesh = geom->GetMesh();

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

		//try to figure out a material
		//try name first, if that fails use index
		std::cout << mesh->mMaterialIndex << std::endl;
		RefCountedPtr<Graphics::Material> mat = model->GetMaterialByIndex(mesh->mMaterialIndex - matIdxOffs);
		//Material names are not consistent throughout formats...
		/*const aiMaterial *amat = scene->mMaterials[mesh->mMaterialIndex];
		aiString s;
		if(AI_SUCCESS == amat->Get(AI_MATKEY_NAME,s)) {
			std::cout << std::string(s.data,s.length) << std::endl;
		}*/

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

		smesh->AddSurface(surface);
	}

	return geom;
}

}
