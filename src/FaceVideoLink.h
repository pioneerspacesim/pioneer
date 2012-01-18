#ifndef _FACEVIDEOLINK
#define _FACEVIDEOLINK

#include "VideoLink.h"

class Texture;

class CharacterInfoText : public Gui::Fixed {
public:
	CharacterInfoText(float w, float h,
		const std::string &name, const std::string &title);
	~CharacterInfoText();
	void GetSizeRequested(float size[2]);
	void Draw();
	std::string const& GetCharacterName() const { return m_characterName; }
	std::string const& GetCharacterTitle() const { return m_characterTitle; }
	void SetCharacterName(const std::string &name);
	void SetCharacterTitle(const std::string &title);
private:
	Gui::Label *m_nameLabel;
	Gui::Label *m_titleLabel;
	Gui::Gradient *m_background;
	std::string m_characterName;
	std::string m_characterTitle;
	float m_width;
	float m_height;
};

class FaceVideoLink : public VideoLink {
public:
	FaceVideoLink(float w, float h, Uint32 flags = 0, Uint32 seed = -1,
		const std::string &name = "", const std::string &title = "");
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
	std::string const& GetCharacterName() const { return m_characterInfo->GetCharacterName(); }
	std::string const& GetCharacterTitle() const { return m_characterInfo->GetCharacterTitle(); }
	void SetCharacterName(const std::string &name) { m_characterInfo->SetCharacterName(name); }
	void SetCharacterTitle(const std::string &title) { m_characterInfo->SetCharacterTitle(title); }

private:
	void DrawMessage();

	Uint32 m_flags;
	Uint32 m_seed;

	Uint32 m_created;
	Texture *m_texture;
	Gui::ToolTip *m_message;
	CharacterInfoText *m_characterInfo;
};

#endif
