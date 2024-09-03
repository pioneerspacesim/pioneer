// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pattern.h"

#include "FileSystem.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"

namespace SceneGraph {

	Pattern::Pattern() :
		smoothColor(false),
		smoothPattern(false),
		name("")
	{
	}

	Pattern::Pattern(const std::string &name_, const std::string &path, Graphics::Renderer *r) :
		smoothColor(false),
		smoothPattern(true),
		name(name_)
	{
		//load as a pattern, allowed:
		//patternNN.png
		//patternNN_n.png
		//patternNN_n_s.png
		//s = smooth (default for pattern)
		//n = nearest (default for color)
		if (name.length() >= 11 && name.compare(10, 1, "n") == 0) smoothPattern = false;
		if (name.length() >= 13 && name.compare(12, 1, "s") == 0) smoothColor = true;

		const std::string patternPath = FileSystem::JoinPathBelow(path, name);

		Graphics::TextureSampleMode sampleMode = smoothPattern ? Graphics::LINEAR_CLAMP : Graphics::NEAREST_CLAMP;
		texture.Reset(Graphics::TextureBuilder(patternPath, sampleMode, true, true, false).GetOrCreateTexture(r, std::string("pattern")));
	}

} // namespace SceneGraph
