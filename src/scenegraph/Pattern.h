// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_PATTERN_H
#define _SCENEGRAPH_PATTERN_H
/*
 * Patterns are a color look-up trick to have customizable
 * colours on models. Patterns are grayscale textures
 * and accompanied by a tiny runtime generated gradient texture
 * with 2-3 colours
 */
#include "RefCounted.h"

#include <vector>
#include <string>

namespace Graphics {
	class Texture;
	class Renderer;
} // namespace Graphics

namespace SceneGraph {

	struct Pattern {
		bool smoothColor;
		bool smoothPattern;
		RefCountedPtr<Graphics::Texture> texture;
		std::string name;

		Pattern();
		Pattern(const std::string &name, const std::string &path, Graphics::Renderer *r);
	};

	typedef std::vector<Pattern> PatternContainer;

} // namespace SceneGraph
#endif
