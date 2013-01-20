// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DEADVIDEOLINK
#define _DEADVIDEOLINK

#include "VideoLink.h"
#include "graphics/Texture.h"
#include "gui/GuiTexturedQuad.h"
#include "RefCounted.h"
#include "SmartPtr.h"

class DeadVideoLink : public VideoLink {
public:
	DeadVideoLink(float w, float h);
	virtual ~DeadVideoLink();

	virtual void Draw();

private:
	void DrawMessage();
	void UpdateWhiteNoise();

	Uint32 m_created;
	Gui::ToolTip *m_message;

	RefCountedPtr<Graphics::Texture> m_texture;
	ScopedPtr<Gui::TexturedQuad> m_quad;
};

#endif
