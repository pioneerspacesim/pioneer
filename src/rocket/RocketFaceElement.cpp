#include "RocketFaceElement.h"

#define FACE_WIDTH  295
#define FACE_HEIGHT 285

// XXX these shouldn't really be hardcoded. it'd be much nicer to poke through
// the facegen/ dir and figure out what we've got available. that or some
// config file
#define MAX_HEAD  20
#define MAX_EYES  20
#define MAX_NOSE  20
#define MAX_MOUTH 20
#define MAX_HAIR  20

#define MAX_CLOTHES     20
#define MAX_ARMOUR      3
#define MAX_ACCESSORIES 16

#define MAX_BACKGROUND 18

static void _blit_image(SDL_Surface *s, const char *filename, int xoff, int yoff)
{
	SDL_Surface *is = IMG_Load(filename);
	if (!is) {
		fprintf(stderr, "RocketFaceElement: couldn't load '%s'\n", filename);
		return;
	}

	SDL_Rect destrec = { ((FACE_WIDTH-is->w-1)/2)+xoff, yoff, 0, 0 };
	SDL_BlitSurface(is, NULL, s, &destrec);
	SDL_FreeSurface(is);
}

RocketFaceElement::~RocketFaceElement()
{
	glDeleteTextures(1, &m_tex);
}

bool RocketFaceElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	dimensions.x = 256.0f;
	dimensions.y = 256.0f;
	return true;
}

void RocketFaceElement::OnAttributeChange(const Rocket::Core::AttributeNameList &changed_attributes)
{
	if (m_initted)
		glDeleteTextures(1, &m_tex);
	
	m_seed = GetAttribute<Uint32>("seed", m_initted ? m_seed : time(0));
	MTRand rand(m_seed);

	m_armour = GetAttribute<bool>("armour", m_initted ? m_armour : false);
	m_gender = GetAttribute<Rocket::Core::String>("gender", m_initted ? (m_gender ? "female" : "male") : (rand.Int32(0,1) ? "female" : "male")) == "female" ? 1 : 0;

	int race = rand.Int32(0,2);

	int head  = rand.Int32(0,MAX_HEAD);
	int eyes  = rand.Int32(0,MAX_EYES);
	int nose  = rand.Int32(0,MAX_NOSE);
	int mouth = rand.Int32(0,MAX_MOUTH);
	int hair  = rand.Int32(0,MAX_HAIR);

	int clothes = rand.Int32(0,MAX_CLOTHES);

	int armour = rand.Int32(0,MAX_ARMOUR);

	int accessories = rand.Int32(0,MAX_ACCESSORIES);

	int back = rand.Int32(0,MAX_BACKGROUND);

	char filename[1024];

	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, ceil_pow2(FACE_WIDTH), ceil_pow2(FACE_HEIGHT), 32, 0xff, 0xff00, 0xff0000, 0xff000000);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/backgrounds/background_%d.png", back);
	_blit_image(s, filename, 0, 0);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/head/head_%d_%d.png", race, m_gender, head);
	_blit_image(s, filename, 0, 0);

	if (!m_armour) {
		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/clothes/cloth_%d_%d.png", m_gender, clothes);
		_blit_image(s, filename, 0, 135);
	}

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/eyes/eyes_%d_%d.png", race, m_gender, eyes);
	_blit_image(s, filename, 0, 41);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/nose/nose_%d_%d.png", race, m_gender, nose);
	_blit_image(s, filename, 1, 89);

	snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/mouth/mouth_%d_%d.png", race, m_gender, mouth);
	_blit_image(s, filename, 0, 155);

	if (!m_armour) {
		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/accessories/acc_%d.png", accessories);
		if (rand.Int32(0,1)>0)	_blit_image(s, filename, 0, 0);

		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/race_%d/hair/hair_%d_%d.png", race, m_gender, hair);
		_blit_image(s, filename, 0, 0);
	}
	else {
		snprintf(filename, sizeof(filename), PIONEER_DATA_DIR "/facegen/clothes/armour_%d.png", armour);
		_blit_image(s, filename, 0, 0);
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);

	glError();

	SDL_FreeSurface(s);

	m_initted = true;
}

void RocketFaceElement::OnRender()
{
	float x1 = GetAbsoluteLeft();
	float y1 = GetAbsoluteTop();
	float x2 = x1 + GetClientWidth();
	float y2 = y1 + GetClientHeight();

	float tw = float(FACE_WIDTH) / ceil_pow2(FACE_WIDTH);
	float th = float(FACE_HEIGHT) / ceil_pow2(FACE_HEIGHT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_QUADS);
		glColor3f(0,0,0);
		glTexCoord2f(0,th);
		glVertex2f(x1,y2);
		glTexCoord2f(tw,th);
		glVertex2f(x2,y2);
		glTexCoord2f(tw,0);
		glVertex2f(x2,y1);
		glTexCoord2f(0,0);
		glVertex2f(x1,y1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


class RocketFaceElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new RocketFaceElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void RocketFaceElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new RocketFaceElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("face", instancer);
	instancer->RemoveReference();
}
