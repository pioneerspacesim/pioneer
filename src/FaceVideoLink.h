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
	void DrawFace();

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;

	Gui::Image *m_head;
	Gui::Image *m_eyes;
	Gui::Image *m_nose;
	Gui::Image *m_mouth;
	Gui::Image *m_hair;
	Gui::Image *m_clothes;
	Gui::Image *m_accessories;
	Gui::Image *m_background;
};

#endif
