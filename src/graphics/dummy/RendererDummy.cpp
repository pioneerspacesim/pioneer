// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RendererDummy.h"

namespace Graphics {

	static Renderer *CreateRenderer(const Settings &vs)
	{
		return new RendererDummy();
	}

	void RendererDummy::RegisterRenderer()
	{
		Graphics::RegisterRenderer(Graphics::RENDERER_DUMMY, CreateRenderer);
	}

} // namespace Graphics
