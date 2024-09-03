// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSkin.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Json.h"
#include "Model.h"
#include "Serializer.h"
#include "StringF.h"
#include "graphics/TextureBuilder.h"

#include "RandomColor.h"

#include <SDL_stdinc.h>

namespace SceneGraph {

	ModelSkin::ModelSkin() :
		m_colors(3)
	{
	}

	void ModelSkin::Apply(Model *model) const
	{
		model->SetColors(m_colors);
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++) {
			if (m_decals[i].empty())
				model->ClearDecal(i);
			else
				model->SetDecalTexture(Graphics::TextureBuilder::Decal(stringf("textures/decals/%0.dds", m_decals[i])).GetOrCreateTexture(model->GetRenderer(), "decal"), i);
		}
		model->SetLabel(m_label);
	}

	void ModelSkin::SetColors(const std::vector<Color> &colors)
	{
		assert(colors.size() == 3);
		m_colors = colors;
	}

	void ModelSkin::SetPrimaryColor(const Color &color)
	{
		m_colors[0] = color;
	}

	void ModelSkin::SetSecondaryColor(const Color &color)
	{
		m_colors[1] = color;
	}

	void ModelSkin::SetTrimColor(const Color &color)
	{
		m_colors[2] = color;
	}

	void ModelSkin::SetRandomColors(Random &rand)
	{
		using namespace RandomColorGenerator;
		static RandomColor s_randomColor;
		m_colors = RandomColor::GetColors(rand, SCHEME_RANDOM, LUMINOSITY_BRIGHT, 3);
	}

	void ModelSkin::SetDecal(const std::string &name, unsigned int index)
	{
		assert(index < MAX_DECAL_MATERIALS);
		m_decals[index] = name;
	}

	void ModelSkin::ClearDecal(unsigned int index)
	{
		assert(index < MAX_DECAL_MATERIALS);
		m_decals[index] = "";
	}

	void ModelSkin::ClearDecals()
	{
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			ClearDecal(i);
	}

	void ModelSkin::SetLabel(const std::string &label)
	{
		m_label = label;
	}

	void ModelSkin::Load(Serializer::Reader &rd)
	{
		for (unsigned int i = 0; i < 3; i++) {
			m_colors[i].r = rd.Byte();
			m_colors[i].g = rd.Byte();
			m_colors[i].b = rd.Byte();
		}
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			m_decals[i] = rd.String();
		m_label = rd.String();
	}

	void ModelSkin::LoadFromJson(const Json &jsonObj)
	{
		try {
			Json modelSkinObj = jsonObj["model_skin"];

			Json colorsArray = modelSkinObj["colors"].get<Json::array_t>();
			if (colorsArray.size() != 3) throw SavedGameCorruptException();
			for (unsigned int i = 0; i < 3; i++) {
				if (colorsArray[i].is_array())
					m_colors[i] = colorsArray[i];
			}

			Json decalsArray = modelSkinObj["decals"].get<Json::array_t>();
			for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++) {
				if (decalsArray.size() > i && decalsArray[i].is_string())
					m_decals[i] = decalsArray[i].get<std::string>();
			}

			m_label = modelSkinObj["label"].get<std::string>();
		} catch (Json::type_error &) {
			throw SavedGameCorruptException();
		}
	}

	void ModelSkin::Save(Serializer::Writer &wr) const
	{
		for (unsigned int i = 0; i < 3; i++) {
			wr.Byte(m_colors[i].r);
			wr.Byte(m_colors[i].g);
			wr.Byte(m_colors[i].b);
		}
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++)
			wr.String(m_decals[i]);
		wr.String(m_label);
	}

	void ModelSkin::SaveToJson(Json &jsonObj) const
	{
		Json modelSkinObj({}); // Create JSON object to contain model skin data.

		Json colorsArray = Json::array(); // Create JSON array to contain colors data.
		for (unsigned int i = 0; i < 3; i++) {
			colorsArray.push_back(m_colors[i]);
		}

		modelSkinObj["colors"] = colorsArray; // Add colors array to model skin object.

		Json decalsArray = Json::array(); // Create JSON array to contain decals data.
		for (unsigned int i = 0; i < MAX_DECAL_MATERIALS; i++) {
			decalsArray.push_back(m_decals[i]);
		}

		modelSkinObj["decals"] = decalsArray; // Add decals array to model skin object.

		modelSkinObj["label"] = m_label;

		jsonObj["model_skin"] = modelSkinObj; // Add model skin object to supplied object.
	}

} // namespace SceneGraph
