#ifndef _FACEVIDEOLINK
#define _FACEVIDEOLINK

#include "VideoLink.h"

class FaceVideoLink : public VideoLink {
public:
	FaceVideoLink(float w, float h, int flags = 0, unsigned long seed = -1);
	virtual ~FaceVideoLink();

	virtual void Draw();

	enum Flags {
		GENDER_RAND   = 0,
		GENDER_MALE   = (1<<0),
		GENDER_FEMALE = (1<<1),
		GENDER_MASK   = 0x03,

		ARMOUR        = (1<<2),
	};

private:
	void DrawMessage();

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;
};

#endif
