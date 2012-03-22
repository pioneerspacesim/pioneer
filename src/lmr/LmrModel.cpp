#include "libs.h"
#include <map>
#include "FontCache.h"
#include "VectorFont.h"
#include "LmrModel.h"
#include "collider/collider.h"
#include "perlin.h"
#include "BufferObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EquipType.h"
#include "EquipSet.h"
#include "ShipType.h"
#include "FileSystem.h"
#include "CRC32.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Shader.h"
#include "graphics/VertexArray.h"
#include "graphics/TextureBuilder.h"
#include "graphics/TextureGL.h" // XXX temporary until LMR uses renderer drawing properly
#include <set>
#include <algorithm>

#include "GeomBuffer.h"
#include "Utils.h"

static const Uint32 s_cacheVersion = 1;

/*
 * Interface: LMR
 *
 * Script interface to the model system.
 *
 * This documentation is incomplete!
 */

static float s_scrWidth = 800.0f;
static std::string s_cacheDir;
static bool s_recompileAllModels = true;

void LmrNotifyScreenWidth(float width)
{
	s_scrWidth = width;
}

static void _fwrite_string(const std::string &str, FILE *f)
{
	int len = str.size()+1;
	fwrite(&len, sizeof(len), 1, f);
	fwrite(str.c_str(), sizeof(char), len, f);
}

static size_t fread_or_die(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	size_t read_count = fread(ptr, size, nmemb, stream);
	if (read_count < nmemb) {
		fprintf(stderr, "Error: failed to read file (%s)\n", (feof(stream) ? "truncated" : "read error"));
		abort();
	}
	return read_count;
}

static std::string _fread_string(FILE *f)
{
	int len = 0;
	fread_or_die(&len, sizeof(len), 1, f);
	char *buf = new char[len];
	fread_or_die(buf, sizeof(char), len, f);
	std::string str = std::string(buf);
	delete[] buf;
	return str;
}

LmrModel::LmrModel(lua_State *lua, const char *model_name, Graphics::Renderer *renderer) : m_lua(lua), m_renderer(renderer)
{
	m_name = model_name;
	m_drawClipRadius = 1.0f;
	m_scale = 1.0f;

	{
	LUA_DEBUG_START(m_lua);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", model_name);
	lua_getglobal(m_lua, buf);
	if (lua_istable(m_lua, -1)) {
		m_numLods = 0;

		lua_getfield(m_lua, -1, "bounding_radius");
		if (lua_isnumber(m_lua, -1)) m_drawClipRadius = luaL_checknumber(m_lua, -1);
		else luaL_error(m_lua, "model %s_info missing bounding_radius=", model_name);
		lua_pop(m_lua, 1);

		lua_getfield(m_lua, -1, "lod_pixels");
		if (lua_istable(m_lua, -1)) {
			for(int i=1;; i++) {
				lua_pushinteger(m_lua, i);
				lua_gettable(m_lua, -2);
				bool is_num = lua_isnumber(m_lua, -1) != 0;
				if (is_num) {
					m_lodPixelSize[i-1] = luaL_checknumber(m_lua, -1);
					m_numLods++;
				}
				lua_pop(m_lua, 1);
				if (!is_num) break;
				if (i > LMR_MAX_LOD) {
					luaL_error(m_lua, "Too many LODs (maximum %d)", LMR_MAX_LOD);
				}
			}
		} else {
			m_numLods = 1;
			m_lodPixelSize[0] = 0;
		}
		lua_pop(m_lua, 1);

		lua_getfield(m_lua, -1, "materials");
		if (lua_istable(m_lua, -1)) {
			for(int i=1;; i++) {
				lua_pushinteger(m_lua, i);
				lua_gettable(m_lua, -2);
				bool is_string = lua_isstring(m_lua, -1) != 0;
				if (is_string) {
					const char *mat_name = luaL_checkstring(m_lua, -1);
					m_materialLookup[mat_name] = m_materials.size();
					m_materials.push_back(LmrMaterial());
				}
				lua_pop(m_lua, 1);
				if (!is_string) break;
			}
		}
		lua_pop(m_lua, 1);

		lua_getfield(m_lua, -1, "scale");
		if (lua_isnumber(m_lua, -1)) {
			m_scale = lua_tonumber(m_lua, -1);
		}
		lua_pop(m_lua, 1);

		/* pop model_info table */
		lua_pop(m_lua, 1);
	} else {
		luaL_error(m_lua, "Could not find function %s_info()", model_name);
	}
	
	snprintf(buf, sizeof(buf), "%s_dynamic", model_name);
	lua_getglobal(m_lua, buf);
	m_hasDynamicFunc = lua_isfunction(m_lua, -1);
	lua_pop(m_lua, 1);

	LUA_DEBUG_END(m_lua, 0);
	}

	for (int i=0; i<m_numLods; i++) {
		m_staticGeometry[i] = new LMR::GeomBuffer(this, true, m_renderer);
		m_dynamicGeometry[i] = new LMR::GeomBuffer(this, false, m_renderer);
	}

	const std::string cache_file = FileSystem::JoinPathBelow(s_cacheDir, model_name) + ".bin";

#if 0
	if (!s_recompileAllModels) {
		// load cached model
		FILE *f = fopen(cache_file.c_str(), "rb");
		if (!f) goto rebuild_model;

		for (int i=0; i<m_numLods; i++) {
			m_staticGeometry[i]->PreBuild();
			m_staticGeometry[i]->LoadFromCache(f);
			m_staticGeometry[i]->PostBuild();
		}
		int numMaterials;
		fread_or_die(&numMaterials, sizeof(numMaterials), 1, f);
		if (size_t(numMaterials) != m_materials.size()) {
			fclose(f);
			goto rebuild_model;
		}
		if (numMaterials) fread_or_die(&m_materials[0], sizeof(LmrMaterial), numMaterials, f);

		int numLights;
		fread_or_die(&numLights, sizeof(numLights), 1, f);
		if (size_t(numLights) != m_lights.size()) {
			fclose(f);
			goto rebuild_model;
		}
		if (numLights) fread_or_die(&m_lights[0], sizeof(LmrLight), numLights, f);

		fclose(f);
	} else {
rebuild_model:
		// run static build for each LOD level
		FILE *f = fopen(cache_file.c_str(), "wb");
#endif
		
		for (int i=0; i<m_numLods; i++) {
			LUA_DEBUG_START(m_lua);
			m_staticGeometry[i]->PreBuild();

			lua_pushlightuserdata(m_lua, m_staticGeometry[i]);
			lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentBuffer");

			lua_pushcfunction(m_lua, pi_lua_panic);
			// call model static building function
			lua_getfield(m_lua, LUA_GLOBALSINDEX, (m_name+"_static").c_str());
			// lod as first argument
			lua_pushnumber(m_lua, i+1);
			lua_pcall(m_lua, 1, 0, -3);
			lua_pop(m_lua, 1);  // remove panic func

			lua_pushnil(m_lua);
			lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentBuffer");

			m_staticGeometry[i]->PostBuild();
// XXX			m_staticGeometry[i]->SaveToCache(f);
			LUA_DEBUG_END(m_lua, 0);
		}
		
#if 0
		const int numMaterials = m_materials.size();
		fwrite(&numMaterials, sizeof(numMaterials), 1, f);
		if (numMaterials) fwrite(&m_materials[0], sizeof(LmrMaterial), numMaterials, f);
		const int numLights = m_lights.size();
		fwrite(&numLights, sizeof(numLights), 1, f);
		if (numLights) fwrite(&m_lights[0], sizeof(LmrLight), numLights, f);
		
		fclose(f);
	}
#endif
}

LmrModel::~LmrModel()
{
	for (int i=0; i<m_numLods; i++) {
		delete m_staticGeometry[i];
		delete m_dynamicGeometry[i];
	}
}

float LmrModel::GetFloatAttribute(const char *attr_name) const
{
	LUA_DEBUG_START(m_lua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(m_lua, buf);
	lua_getfield(m_lua, -1, attr_name);
	float result = luaL_checknumber(m_lua, -1);
	lua_pop(m_lua, 2);
	LUA_DEBUG_END(m_lua, 0);
	return result;
}

int LmrModel::GetIntAttribute(const char *attr_name) const
{
	LUA_DEBUG_START(m_lua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(m_lua, buf);
	lua_getfield(m_lua, -1, attr_name);
	int result = luaL_checkinteger(m_lua, -1);
	lua_pop(m_lua, 2);
	LUA_DEBUG_END(m_lua, 0);
	return result;
}

bool LmrModel::GetBoolAttribute(const char *attr_name) const
{
	char buf[256];
	LUA_DEBUG_START(m_lua);
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(m_lua, buf);
	lua_getfield(m_lua, -1, attr_name);
	bool result;
	if (lua_isnil(m_lua, -1)) {
		result = false;
	} else {
		result = lua_toboolean(m_lua, -1) != 0;
	}	
	lua_pop(m_lua, 2);
	LUA_DEBUG_END(m_lua, 0);
	return result;
}

void LmrModel::PushAttributeToLuaStack(const char *attr_name) const
{
	LUA_DEBUG_START(m_lua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(m_lua, buf);
	lua_getfield(m_lua, -1, attr_name);
	lua_remove(m_lua, -2);
	LUA_DEBUG_END(m_lua, 1);
}

bool LmrModel::HasTag(const char *tag) const
{
	bool has_tag = false;

	LUA_DEBUG_START(m_lua);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());

	lua_getglobal(m_lua, buf);
	lua_getfield(m_lua, -1, "tags");
	if (lua_istable(m_lua, -1)) {
		for(int j=1;; j++) {
			lua_pushinteger(m_lua, j);
			lua_gettable(m_lua, -2);
			if (lua_isstring(m_lua, -1)) {
				const char *s = luaL_checkstring(m_lua, -1);
				if (0 == strcmp(tag, s)) {
					has_tag = true;
					lua_pop(m_lua, 1);
					break;
				}
			} else if (lua_isnil(m_lua, -1)) {
				lua_pop(m_lua, 1);
				break;
			}
			lua_pop(m_lua, 1);
		}
	}
	lua_pop(m_lua, 2);

	LUA_DEBUG_END(m_lua, 0);

	return has_tag;
}

void LmrModel::Render(const matrix4x4f &trans, const LmrObjParams *params)
{
	LMR::RenderState rstate;
	rstate.subTransform = matrix4x4f::Identity();
	rstate.combinedScale = m_scale;
	Render(&rstate, vector3f(-trans[12], -trans[13], -trans[14]), trans, params);
}

void LmrModel::Render(const LMR::RenderState *rstate, const vector3f &cameraPos, const matrix4x4f &trans, const LmrObjParams *params)
{
	glPushMatrix();
	glMultMatrixf(&trans[0]);
	glScalef(m_scale, m_scale, m_scale);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);

	float pixrad = 0.5f * s_scrWidth * rstate->combinedScale * m_drawClipRadius / cameraPos.Length();
	//printf("%s: %fpx\n", m_name.c_str(), pixrad);

	int lod = m_numLods-1;
	for (int i=lod-1; i>=0; i--) {
		if (pixrad < m_lodPixelSize[i]) lod = i;
	}
	//printf("%s: lod %d\n", m_name.c_str(), lod);

	Build(lod, params);

	const vector3f modelRelativeCamPos = trans.InverseOf() * cameraPos;

	m_staticGeometry[lod]->Render(rstate, modelRelativeCamPos, params);
	if (m_hasDynamicFunc) {
		m_dynamicGeometry[lod]->Render(rstate, modelRelativeCamPos, params);
	}

	Graphics::UnbindAllBuffers();

	glDisable(GL_NORMALIZE);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	glPopMatrix();
}

void LmrModel::Build(int lod, const LmrObjParams *params)
{
	if (m_hasDynamicFunc) {
		LUA_DEBUG_START(m_lua);
		m_dynamicGeometry[lod]->PreBuild();

		lua_pushlightuserdata(m_lua, m_dynamicGeometry[lod]);
		lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentBuffer");
		lua_pushlightuserdata(m_lua, const_cast<LmrObjParams*>(params));
		lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentParams");

		lua_pushcfunction(m_lua, pi_lua_panic);
		// call model dynamic bits
		lua_getfield(m_lua, LUA_GLOBALSINDEX, (m_name+"_dynamic").c_str());
		// lod as first argument
		lua_pushnumber(m_lua, lod+1);
		lua_pcall(m_lua, 1, 0, -3);
		lua_pop(m_lua, 1);  // remove panic func

		lua_pushnil(m_lua);
		lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentBuffer");
		lua_pushnil(m_lua);
		lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiLmrCurrentParams");

		m_dynamicGeometry[lod]->PostBuild();
		LUA_DEBUG_END(m_lua, 0);
	}
}

void LmrModel::GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform, const LmrObjParams *params)
{
	// use lowest LOD
	Build(0, params);
	matrix4x4f m = transform * matrix4x4f::ScaleMatrix(m_scale);
	m_staticGeometry[0]->GetCollMeshGeometry(mesh, m, params);
	if (m_hasDynamicFunc) m_dynamicGeometry[0]->GetCollMeshGeometry(mesh, m, params);
}

LmrCollMesh::LmrCollMesh(LmrModel *m, const LmrObjParams *params)
{
#if 0
	memset(this, 0, sizeof(LmrCollMesh));
	m_aabb.min = vector3d(DBL_MAX, DBL_MAX, DBL_MAX);
	m_aabb.max = vector3d(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	m->GetCollMeshGeometry(this, matrix4x4f::Identity(), params);
	geomTree = new GeomTree(nv, m_numTris, pVertex, pIndex, pFlag);
#endif
	// XXX no collision meshes until we sort out the GeomBuffer internals
	// for now a fixed radius so the modelviewer can do something useful
	m_radius = 1.0;
}

/** returns number of tris found (up to 'num') */
int LmrCollMesh::GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const
{
	int found = 0;
	for (int i=0; (i<m_numTris) && (found<num); i++) {
		if (pFlag[i] == flags) {
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i]]);
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i+1]]);
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i+2]]);
			found++;
		}
	}
	return found;
}

LmrCollMesh::~LmrCollMesh()
{
	// nice. mixed allocation. for the love of realloc...
#if 0
	delete geomTree;
	free(pVertex);
	free(pIndex);
	free(pFlag);
#endif
}


static Uint32 s_allModelFilesCRC;

static Uint32 _calculate_all_models_checksum()
{
	// do we need to rebuild the model cache?
	CRC32 crc;
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "models", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		if (info.IsFile() && (info.GetPath().substr(info.GetPath().size() - 4) != ".png")) {
			RefCountedPtr<FileSystem::FileData> data = files.Current().Read();
			crc.AddData(data->GetData(), data->GetSize());
		}
	}
	return crc.GetChecksum();
}

static void _detect_model_changes()
{
	s_allModelFilesCRC = _calculate_all_models_checksum();

	FILE *cache_sum_file = fopen(FileSystem::JoinPath(s_cacheDir, "cache.sum").c_str(), "rb");
	if (cache_sum_file) {
		Uint32 version;
		fread_or_die(&version, sizeof(version), 1, cache_sum_file);
		if (version == s_cacheVersion) {
			Uint32 checksum;
			fread_or_die(&checksum, sizeof(checksum), 1, cache_sum_file);
			if (checksum == s_allModelFilesCRC) {
				s_recompileAllModels = false;
			}
		}
		fclose(cache_sum_file);
	}
	if (s_recompileAllModels) printf("Rebuilding model cache...\n");
}

static void _write_model_crc_file()
{
	if (s_recompileAllModels) {
		FILE *cache_sum_file = fopen(FileSystem::JoinPath(s_cacheDir, "cache.sum").c_str(), "wb");
		if (cache_sum_file) {
			fwrite(&s_cacheVersion, sizeof(s_cacheVersion), 1, cache_sum_file);
			fwrite(&s_allModelFilesCRC, sizeof(s_allModelFilesCRC), 1, cache_sum_file);
			fclose(cache_sum_file);
		}
	}
}

