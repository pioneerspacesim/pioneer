// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BaseLoader.h"
#include "FileSystem.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "scenegraph/LoaderDefinitions.h"
#include "scenegraph/Parser.h"
#include "utils.h"

#include "lz4/xxhash.h"

using namespace SceneGraph;

BaseLoader::BaseLoader(Graphics::Renderer *r) :
	m_renderer(r),
	m_model(nullptr)
{
	Graphics::Texture *sdfTex = Graphics::TextureBuilder("fonts/label3d.dds",
		Graphics::LINEAR_CLAMP, true, true, true)
									.GetOrCreateTexture(r, "model");
	m_labelFont.Reset(new Text::DistanceFieldFont("fonts/sdf_definition.txt", sdfTex));
}

RefCountedPtr<Graphics::Material> BaseLoader::GetMaterialForMesh(std::string_view name, const Graphics::VertexFormatDesc &vtxFormat)
{
	size_t vtxHash = vtxFormat.Hash();

	// Intern the vertex format for debugging purposes
	m_vtxFormatCache.try_emplace(vtxHash, vtxFormat);

	// Create a hash key from the material name and the vertex format hash
	uint64_t materialKey = XXH64(name.data(), name.size(), vtxHash);
	auto iter = m_materialLookup.find(materialKey);

	// If we already have the material cached, we can just use it.
	if (iter != m_materialLookup.end()) {
		return iter->second;
	}

	// Create a new material instance according to the material definition
	auto mdef = std::find_if(m_modelDef->matDefs.begin(), m_modelDef->matDefs.end(), [&](const MaterialDefinition &v) {
		return name.compare(v.name) == 0;
	});

	if (mdef == m_modelDef->matDefs.end()) {
		Log::Warning("{}: No material definition found for material name {}, using material {}",
			m_modelDef->name, name, m_modelDef->matDefs.front().name);
		mdef = m_modelDef->matDefs.begin();
	}

	RefCountedPtr<Graphics::Material> mat = ConvertMaterialDefinition(*mdef, vtxFormat);

	m_model->m_materials.push_back(std::make_pair(mdef->name, mat));
	m_materialLookup.try_emplace(materialKey, mat);

	return mat;
}

RefCountedPtr<Graphics::Material> BaseLoader::ConvertMaterialDefinition(const MaterialDefinition &mdef, const Graphics::VertexFormatDesc &vtxFormat)
{
	//Build material descriptor
	Graphics::MaterialDescriptor matDesc;
	matDesc.lighting = !mdef.unlit;
	matDesc.alphaTest = mdef.alpha_test;

	matDesc.usePatterns = mdef.use_patterns;

	//diffuse texture is a must. Will create a white dummy texture if one is not supplied
	matDesc.textures = 1;

	// TODO: kill with fire. Explicit variant selection via a `technique "XXYYZZ"` directive?
	for (const auto &bind : mdef.textureBinds) {
		if (bind.first == "specular")
			matDesc.specularMap = true;
		if (bind.first == "glow")
			matDesc.glowMap = true;
		if (bind.first == "ambient")
			matDesc.ambientMap = true;
		if (bind.first == "normal")
			matDesc.normalMap = true;
	}

	matDesc.quality = Graphics::HAS_HEAT_GRADIENT;

	//Create material and set parameters
	RefCountedPtr<Graphics::Material> mat(m_renderer->CreateMaterial(mdef.shader, matDesc, mdef.renderState, vtxFormat));
	mat->diffuse = mdef.diffuse;
	mat->specular = mdef.specular;
	mat->emissive = mdef.emissive;
	mat->shininess = mdef.shininess;

	//semitransparent material
	//the node must be marked transparent when using this material
	//and should not be mixed with opaque materials
	if (mdef.opacity < 100)
		mat->diffuse.a = (float(mdef.opacity) / 100.f) * 255;

	// Fall back to an empty white texture if no textures specified.
	if (mdef.textureBinds.empty()) {
		Graphics::Texture *tex = Graphics::TextureBuilder::GetWhiteTexture(m_renderer);
		mat->SetTexture("diffuse"_hash, tex);
	}

	// Bind textures to the material instance
	for (const auto &bind : mdef.textureBinds) {
		Graphics::Texture *tex = Graphics::TextureBuilder::Model(bind.second).GetOrCreateTexture(m_renderer, "model");
		if (!mat->SetTexture(Graphics::Renderer::GetName(bind.first), tex)) {
			Log::Warning("Model {}: Binding texture {} to slot {} failed for material {} (invalid slot).",
				m_modelDef->name, bind.second, bind.first, mdef.name);
		}
	}

	return mat;
}

RefCountedPtr<Graphics::Material> BaseLoader::GetDecalMaterial(unsigned int index)
{
	assert(index <= Model::MAX_DECAL_MATERIALS);
	RefCountedPtr<Graphics::Material> &decMat = m_model->m_decalMaterials[index - 1];

	if (!decMat.Valid()) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		matDesc.lighting = true;

		Graphics::RenderStateDesc rsd;
		rsd.depthWrite = false;
		rsd.blendMode = Graphics::BLEND_ALPHA;

		// Hardcoding the vertex format here to work around the design of decal materials in Model
		// Ideally decals should be specified as a texture and a model-wide bind-group be updated
		Graphics::VertexFormatDesc vtxFormat = Graphics::VertexFormatDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0 | Graphics::ATTRIB_TANGENT);

		// XXX add depth bias to render state parameter
		decMat.Reset(m_renderer->CreateMaterial("multi", matDesc, rsd, vtxFormat));
		decMat->SetTexture("texture0"_hash,
			Graphics::TextureBuilder::GetTransparentTexture(m_renderer));
		decMat->specular = Color::BLACK;
		decMat->diffuse = Color::WHITE;
	}

	return decMat;
}

ModelDefinition *BaseLoader::LoadModelDefinition(std::string_view path)
{
	FileSystem::FileSource &fs = FileSystem::gameDataFiles;

	RefCountedPtr<FileSystem::FileData> filedata = fs.ReadFile(std::string(path));
	if (!filedata) {
		Log::Warning("LoadModelDefinition: {}: could not read file\n", path);
		return nullptr;
	}

	const FileSystem::FileInfo &info = filedata->GetInfo();

	//curPath is used to find textures, patterns,
	//possibly other data files for this model.
	//Strip trailing slash
	m_curPath = info.GetDir();
	assert(!m_curPath.empty());

	if (m_curPath[m_curPath.length() - 1] == '/')
		m_curPath = m_curPath.substr(0, m_curPath.length() - 1);

	ModelDefinition *modelDefinition = new ModelDefinition();

	if (starts_with_ci(filedata->AsStringView(), "version 2")) {

		ParserV2 parser = {};
		bool ok = parser.Parse(*filedata, modelDefinition);

		if (!ok) {
			delete modelDefinition;
			throw LoadingError("Failed to parse!");
		}

	} else {

		try {
			Parser p(fs, std::string(path), m_curPath);
			p.Parse(modelDefinition);
		} catch (ParseError &err) {
			Output("%s\n", err.what());
			delete modelDefinition;
			throw LoadingError(err.what());
		}

	}

	// Set name to filename minus extension
	const std::string &filename = info.GetName();
	modelDefinition->name = filename.substr(0, filename.find_last_of('.'));

	return modelDefinition;
}

void BaseLoader::FindPatterns(PatternContainer &output)
{
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, m_curPath); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		if (info.IsFile()) {
			const std::string &name = info.GetName();
			if (starts_with(name, "pattern")) {
				if (ends_with_ci(name, ".png") || ends_with_ci(name, ".dds"))
					output.push_back(Pattern(name, m_curPath, m_renderer));
			}
		}
	}
}

void BaseLoader::SetUpPatterns()
{
	FindPatterns(m_model->m_patterns);

	if (m_model->m_patterns.empty()) {
		m_model->m_patterns.push_back(Pattern());
		Pattern &dumpat = m_model->m_patterns.back();
		dumpat.name = "Dummy";
		dumpat.texture = RefCountedPtr<Graphics::Texture>(Graphics::TextureBuilder::GetWhiteTexture(m_renderer));
	}

	//set up some noticeable default colors
	std::vector<Color> colors;
	colors.push_back(Color::RED);
	colors.push_back(Color::GREEN);
	colors.push_back(Color::BLUE);
	m_model->SetColors(colors);
	m_model->SetPattern(0);
}
