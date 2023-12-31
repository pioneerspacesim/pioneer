// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SCENEGRAPH_MODELSKIN_H
#define SCENEGRAPH_MODELSKIN_H

#include "Color.h"
#include "JsonFwd.h"
#include "Random.h"
#include "lua/LuaWrappable.h"
#include <string>

namespace Serializer {
	class Reader;
	class Writer;
} // namespace Serializer

namespace SceneGraph {

	class Model;

	class ModelSkin : public LuaWrappable {
	public:
		ModelSkin();

		void Apply(Model *model) const;

		void SetColors(const std::vector<Color> &colors);
		void SetPrimaryColor(const Color &color);
		void SetSecondaryColor(const Color &color);
		void SetTrimColor(const Color &color);
		void SetRandomColors(Random &rand);

		void SetDecal(const std::string &name, unsigned int index = 0);
		void ClearDecal(unsigned int index = 0);
		void ClearDecals();

		void SetLabel(const std::string &label);

		void Load(Serializer::Reader &rd);
		void LoadFromJson(const Json &jsonObj);
		void Save(Serializer::Writer &wr) const;
		void SaveToJson(Json &jsonObj) const;

		const std::vector<Color> &GetColors() const { return m_colors; }

	private:
		static const unsigned int MAX_DECAL_MATERIALS = 4;

		std::vector<Color> m_colors;
		std::string m_decals[MAX_DECAL_MATERIALS];
		std::string m_label;
	};

} // namespace SceneGraph

#endif
