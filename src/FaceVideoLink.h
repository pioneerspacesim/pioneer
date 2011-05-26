#ifndef _FACEVIDEOLINK
#define _FACEVIDEOLINK

#include "VideoLink.h"

class FaceVideoLink : public VideoLink {
public:
	FaceVideoLink(float w, float h, Uint32 flags = 0, Uint32 seed = -1);
	virtual ~FaceVideoLink();

	virtual void Draw();

	enum Flags {
		GENDER_RAND   = 0,
		GENDER_MALE   = (1<<0),
		GENDER_FEMALE = (1<<1),
		GENDER_MASK   = 0x03,

		ARMOUR        = (1<<2),
	};

    Uint32 GetFlags() const { return m_flags; }
    Uint32 GetSeed() const { return m_seed; }

private:
	void DrawMessage();

	Uint32 m_flags;
	Uint32 m_seed;

	Uint32 m_created;
	GLuint m_tex;
	Gui::ToolTip *m_message;
};

#endif
