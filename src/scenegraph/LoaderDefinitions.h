// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LOADERDEFINITIONS_H_
#define _LOADERDEFINITIONS_H_
/*
 * Data strcutrures used by Loader
 */
#include "Color.h"
#include "../vector3.h"
#include "graphics/RenderState.h"

#include <string>
#include <vector>
#include <utility>

namespace SceneGraph {

	struct LodDefinition {
		LodDefinition(float size) :
			pixelSize(size)
		{}
		float pixelSize;
		std::vector<std::string> meshNames;
	};

	struct AnimDefinition {
		AnimDefinition(const std::string &name_, double start_, double end_, bool loop_) :
			name(name_),
			start(start_),
			end(end_),
			loop(loop_)
		{}
		std::string name;
		double start;
		double end;
		bool loop;
	};

	struct BoundDefinition {
		enum Type {
			// Capsule (cylinder with rounded end-caps)
			// startTag, endTag define the endpoints of the central line of the cylinder
			// radius defines both the radius of the cylinder and the radius of the endcaps
			CAPSULE,
		};
		Type type;
		std::string startTag;
		std::string endTag;
		double radius;

		// What boundary does this bound definition refer to?
		std::string forBound;

		static BoundDefinition create_capsule(const std::string &for_b, const std::string &start,
			const std::string &end, const double rad) {
			BoundDefinition out;
			out.type = CAPSULE;
			out.startTag = start;
			out.endTag = end;
			out.radius = rad;
			out.forBound = for_b;
			return out;
		}

	};

	struct MaterialDefinition {
		std::string name;
		std::string shader;
		std::vector<std::pair<std::string, std::string>> textureBinds;

		Color diffuse = Color::WHITE;
		Color specular = Color::WHITE;
		Color ambient = Color::BLANK;
		Color emissive = Color::BLANK;
		float shininess = 100;
		float opacity = 100;
		Graphics::RenderStateDesc renderState;
		bool unlit = false;
		bool use_patterns = false;
		bool alpha_test = false;
	};

	struct ModelDefinition {
		std::string name;
		std::vector<LodDefinition> lodDefs;
		std::vector<MaterialDefinition> matDefs;
		std::vector<std::string> collisionDefs;
		std::vector<AnimDefinition> animDefs;
		std::vector<BoundDefinition> boundsDefs;
	};

} // namespace SceneGraph

#endif
