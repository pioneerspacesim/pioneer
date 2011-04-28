#ifndef _FACEVIDEOLINK
#define _FACEVIDEOLINK

#include "VideoLink.h"

class FaceVideoLink : public VideoLink {
public:
	FaceVideoLink(float w, float h, unsigned long seed);
	virtual ~FaceVideoLink();

	virtual void Draw();

private:
	void DrawMessage();

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;
};

#endif
