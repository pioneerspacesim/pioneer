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
