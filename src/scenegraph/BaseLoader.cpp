// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BaseLoader.h"
#include "FileSystem.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "utils.h"

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

void BaseLoader::ConvertMaterialDefinition(const MaterialDefinition &mdef)
{
	//Build material descriptor
	const std::string &diffTex = mdef.tex_diff;
	const std::string &specTex = mdef.tex_spec;
	const std::string &glowTex = mdef.tex_glow;
	const std::string &ambiTex = mdef.tex_ambi;
	const std::string &normTex = mdef.tex_norm;

	Graphics::MaterialDescriptor matDesc;
	matDesc.lighting = !mdef.unlit;
	matDesc.alphaTest = mdef.alpha_test;

	matDesc.usePatterns = mdef.use_pattern;

	//diffuse texture is a must. Will create a white dummy texture if one is not supplied
	matDesc.textures = 1;
	matDesc.specularMap = !specTex.empty();
	matDesc.glowMap = !glowTex.empty();
	matDesc.ambientMap = !ambiTex.empty();
	matDesc.normalMap = !normTex.empty();
	matDesc.quality = Graphics::HAS_HEAT_GRADIENT;

	// FIXME: add render state properties to MaterialDefinition
	// This is hacky and based off of the code in Loader.cpp
	Graphics::RenderStateDesc rsd;
	if (mdef.opacity < 100) {
		rsd.blendMode = Graphics::BLEND_ALPHA;
		rsd.depthWrite = false;
	}

	//Create material and set parameters
	RefCountedPtr<Graphics::Material> mat(m_renderer->CreateMaterial("multi", matDesc, rsd));
	mat->diffuse = mdef.diffuse;
	mat->specular = mdef.specular;
	mat->emissive = mdef.emissive;
	mat->shininess = mdef.shininess;

	//semitransparent material
	//the node must be marked transparent when using this material
	//and should not be mixed with opaque materials
	if (mdef.opacity < 100)
		mat->diffuse.a = (float(mdef.opacity) / 100.f) * 255;

	Graphics::Texture *texture0 = nullptr;
	Graphics::Texture *texture1 = nullptr;
	Graphics::Texture *texture2 = nullptr;
	Graphics::Texture *texture3 = nullptr;
	Graphics::Texture *texture6 = nullptr;
	if (!diffTex.empty())
		texture0 = Graphics::TextureBuilder::Model(diffTex).GetOrCreateTexture(m_renderer, "model");
	else
		texture0 = Graphics::TextureBuilder::GetWhiteTexture(m_renderer);
	if (!specTex.empty())
		texture1 = Graphics::TextureBuilder::Model(specTex).GetOrCreateTexture(m_renderer, "model");
	if (!glowTex.empty())
		texture2 = Graphics::TextureBuilder::Model(glowTex).GetOrCreateTexture(m_renderer, "model");
	if (!ambiTex.empty())
		texture3 = Graphics::TextureBuilder::Model(ambiTex).GetOrCreateTexture(m_renderer, "model");
	//texture4 is reserved for pattern
	//texture5 is reserved for color gradient
	if (!normTex.empty())
		texture6 = Graphics::TextureBuilder::Normal(normTex).GetOrCreateTexture(m_renderer, "model");

	mat->SetTexture("texture0"_hash, texture0);
	mat->SetTexture("texture1"_hash, texture1);
	mat->SetTexture("texture2"_hash, texture2);
	mat->SetTexture("texture3"_hash, texture3);
	mat->SetTexture("texture6"_hash, texture6);

	m_model->m_materials.push_back(std::make_pair(mdef.name, mat));
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

		// XXX add depth bias to render state parameter
		decMat.Reset(m_renderer->CreateMaterial("multi", matDesc, rsd));
		decMat->SetTexture("texture0"_hash,
			Graphics::TextureBuilder::GetTransparentTexture(m_renderer));
		decMat->specular = Color::BLACK;
		decMat->diffuse = Color::WHITE;
	}
	return decMat;
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
