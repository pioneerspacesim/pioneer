#ifndef _FACEVIDEOLINK
#define _FACEVIDEOLINK

#include "VideoLink.h"

class FaceVideoLink : public VideoLink {
public:
	FaceVideoLink(float w, float h);
	virtual ~FaceVideoLink();

	virtual void Draw();

private:
	void DrawMessage();
	void DrawFace();

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;

	Gui::Image *m_face;
	Gui::Image *m_eyes;
	Gui::Image *m_nose;
	Gui::Image *m_hair;
	Gui::Image *m_mouth;
	Gui::Image *m_cloth;
	Gui::Image *m_extr1;
	Gui::Image *m_extr2;
};

#endif
