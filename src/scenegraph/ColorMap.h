// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_COLORMAP_H
#define _SCENEGRAPH_COLORMAP_H
/*
 * Color look-up texture generator for newmodel pattern system
 */
#include "libs.h"
#include "graphics/Texture.h"

namespace Graphics { class Renderer; }

namespace SceneGraph {

class ColorMap {
public:
	ColorMap();
	Graphics::Texture *GetTexture();
	void Generate(Graphics::Renderer *r, const Color4ub &a, const Color4ub &b, const Color4ub &c);
	void SetSmooth(bool);

private:
	void AddColor(int width, const Color4ub &c, std::vector<unsigned char> &out);

	bool m_smooth;
	RefCountedPtr<Graphics::Texture> m_texture;
};

}

#endif
