#ifndef _DEADVIDEOLINK
#define _DEADVIDEOLINK

#include "VideoLink.h"

class DeadVideoLink : public VideoLink {
public:
	DeadVideoLink(float w, float h);
	virtual ~DeadVideoLink();

	virtual void Draw();

private:
	void DrawMessage();
	void PutRandomCrapIntoTexture();

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;
};

#endif
