#include "Loader.h"
#include "LuaUtils.h"
#include "Newmodel.h"
#include "FileSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>
#include <assimp/material.h>
#include "StaticGeometry.h"

namespace Newmodel {

static ModelDefinition *g_curModel; //XXX any better way?
static std::string g_curName;

//XXX copied from ShipType
static void _get_string_attrib(lua_State *L, const char *key, std::string &output,
		const char *default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tostring(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static int define_model(lua_State *L)
{
	//get material definitions
	lua_pushstring(L, "materials");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		//each definition is a table
		for (unsigned int i=0; i<lua_objlen(L, -1); i++) {
			MaterialDefinition matdef;
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			if(lua_istable(L, -1)) {
				_get_string_attrib(L, "name", matdef.name, "");
				if (matdef.name.empty()) luaL_error(L, "Material has no name");
				_get_string_attrib(L, "diffuse", matdef.diffuseTexture, "");
			}
			lua_pop(L, 1);
			g_curModel->matDefs.push_back(matdef);
		}
	}
	lua_pop(L, 1);
	//printf("Defined %d materials\n", g_curModel->matDefs.size());

	//get mesh names
	lua_pushstring(L, "meshes");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		for (unsigned int i=0; i<lua_objlen(L,-1); i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			std::string mesh = luaL_checkstring(L,-1);
			if (!mesh.empty())
				g_curModel->meshNames.push_back(mesh);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	//printf("Defined %d meshes\n", g_curModel->meshNames.size());
	return 0;
}

Loader::Loader(Graphics::Renderer *r)
{
	m_luaState = lua_open();
	luaL_openlibs(m_luaState);
	lua_register(m_luaState, "model", define_model);
}

Loader::~Loader()
{
	lua_close(m_luaState);
}

NModel *Loader::LoadModel(const std::string &filename)
{
	//XXX not recursive
	static const std::string basepath("newmodels");
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirectories); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();
		//check it's the expected type
		if (info.IsFile() && (fpath.substr(fpath.find_last_of(".")+1) == "model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();
			if (filename == name.substr(0, name.length()-6)) {
				ModelDefinition modelDefinition;
				g_curModel = &modelDefinition;
				g_curName = name.substr(0, name.length()-6);
				pi_lua_dofile(m_luaState, fpath);
				g_curName.clear();
				g_curModel = 0;
				//XXX hmm
				m_curPath = info.GetDir();
				m_curPath = FileSystem::JoinPath("data", m_curPath);
				return CreateModel(modelDefinition);
			}
		}
		
	}
	throw LoadingError();
}

NModel *Loader::CreateModel(const ModelDefinition &def)
{
	if (def.meshNames.empty()) return 0;

	NModel *model = new NModel(def.name);
	//load meshes
	for(std::vector<std::string>::const_iterator it = def.meshNames.begin();
		it != def.meshNames.end(); ++it)
	{
		try {
			Node *mesh = LoadMesh(FileSystem::JoinPathBelow(m_curPath, *(it)));
			model->GetRoot()->AddChild(mesh);
		} catch (LoadingError &) {
			delete model;
			throw;
		}
	}
	return model;
}

Node *Loader::LoadMesh(const std::string &filename)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filename, aiProcess_OptimizeGraph | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals );

	if(!scene)
		throw LoadingError();

	StaticGeometry *geom = new StaticGeometry();

	//dummy material
	RefCountedPtr<Graphics::Material> mat(new Graphics::Material());
	Graphics::StaticMesh *smesh = geom->GetMesh();

	//turn meshes into surfaces
	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

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
			for (unsigned int i = 0; i < face->mNumIndices; i++) {
				indices.push_back(face->mIndices[i]);
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