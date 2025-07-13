// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_LOADER_H
#define _SCENEGRAPH_LOADER_H
/**
 * Model loader baseclass
 */
#include "LoaderDefinitions.h"
#include "Model.h"
#include "StaticGeometry.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "text/DistanceFieldFont.h"

namespace SceneGraph {

	class BaseLoader {
	public:
		BaseLoader(Graphics::Renderer *r);

		Graphics::Renderer *GetRenderer() const { return m_renderer; }
		RefCountedPtr<Text::DistanceFieldFont> GetLabel3DFont() const { return m_labelFont; }

		//allocate material for dynamic decal, should be used in order 1..4
		//TODO: vertex format for decal meshes is currently hardcoded
		RefCountedPtr<Graphics::Material> GetDecalMaterial(unsigned int index);
		RefCountedPtr<Graphics::Material> GetMaterialForMesh(std::string_view name, const Graphics::VertexFormatDesc &vtxFormat);

	protected:
		Graphics::Renderer *m_renderer;
		Model *m_model;
		ModelDefinition *m_modelDef;
		std::string m_curPath; //path of current model file
		RefCountedPtr<Text::DistanceFieldFont> m_labelFont;

		std::map<uint64_t, Graphics::VertexFormatDesc> m_vtxFormatCache;
		std::map<uint64_t, RefCountedPtr<Graphics::Material>> m_materialLookup;

		//create a material from definition and add it to m_model
		RefCountedPtr<Graphics::Material> ConvertMaterialDefinition(const MaterialDefinition &, const Graphics::VertexFormatDesc &vtxFormat);


		//find pattern texture files from the model directory
		void FindPatterns(PatternContainer &output);
		void SetUpPatterns();
	};

} // namespace SceneGraph
#endif
