#ifndef _SHIPCPANELMULTIFUNCDISPLAYS_H
#define _SHIPCPANELMULTIFUNCDISPLAYS_H

#include "gui/Gui.h"
#include "EquipType.h"
#include "Serializer.h"
#include "Object.h"

class Body;
namespace Graphics { class Renderer; }

enum multifuncfunc_t {
	MFUNC_SCANNER,
	MFUNC_EQUIPMENT,
	MFUNC_MSGLOG,
	MFUNC_MAX
};

class IMultiFunc {
public:
	sigc::signal<void> onGrabFocus;
	sigc::signal<void> onUngrabFocus;
	virtual void Update() = 0;
};

class MsgLogWidget: public IMultiFunc, public Gui::Fixed {
public:
	MsgLogWidget();
	void GetSizeRequested(float size[2]);

	void ImportantMessage(const std::string &sender, const std::string &msg) {
		m_msgQueue.push_back(message_t(sender, msg, MUST_SEE));
	}
	void Message(const std::string &sender, const std::string &msg) {
		m_msgQueue.push_back(message_t(sender, msg, NOT_IMPORTANT));
	}
	virtual void Update();
private:
	enum Type {
		NONE = -1,
		NOT_IMPORTANT = 0,
		MUST_SEE = 1
	};
	void ShowNext();
	struct message_t {
		message_t(std::string s, std::string m, Type t): sender(s), message(m), type(t) {}
		std::string sender;
		std::string message;
		Type type;
	};
	std::list<message_t> m_msgQueue;
	Uint32 m_msgAge;
	Gui::Label *m_msgLabel;
	Type m_curMsgType;
};

class ScannerWidget: public IMultiFunc, public Gui::Widget {
public:
	ScannerWidget(Graphics::Renderer *r);
	ScannerWidget(Graphics::Renderer *r, Serializer::Reader &rd);
	virtual ~ScannerWidget();
	void GetSizeRequested(float size[2]);
	void ToggleMode();
	void Draw();
	virtual void Update();

	void TimeStepUpdate(float step);

	void Save(Serializer::Writer &wr);

private:
	void InitObject();

	void DrawBlobs(bool below);
	void DrawRingsAndSpokes(bool blend);

	sigc::connection m_toggleScanModeConnection;

	struct Contact {
		Object::Type type;
		vector3d pos;
		bool isSpecial;
	};
	std::list<Contact> m_contacts;

	enum ScannerMode { SCANNER_MODE_AUTO, SCANNER_MODE_MANUAL };
	ScannerMode m_mode;

	float m_currentRange, m_manualRange, m_targetRange;
	float m_scale;

	float m_x;
	float m_y;

	Graphics::Renderer *m_renderer;
};

class UseEquipWidget: public IMultiFunc, public Gui::Fixed {
public:
	UseEquipWidget();
	virtual ~UseEquipWidget();
	void GetSizeRequested(float size[2]);
	virtual void Update() {}
private:
	void UpdateEquip();
	void UseRadarMapper();
	void UseHypercloudAnalyzer();
	enum { MAX_MISSILE_SLOTS = 8 };

	sigc::connection m_onPlayerEquipChangedCon;

	void FireMissile(int idx);
};


class MultiFuncSelectorWidget: public Gui::Fixed {
public:
	MultiFuncSelectorWidget();
	virtual ~MultiFuncSelectorWidget();
	sigc::signal<void, multifuncfunc_t> onSelect;
	void SetSelected(multifuncfunc_t f) {
		m_rg->SetSelected(int(f));
	}
private:
	void UpdateButtons();
	void OnClickButton(multifuncfunc_t f);

	int m_active;
	Gui::ImageRadioButton *m_buttons[MFUNC_MAX];
	Gui::RadioGroup *m_rg;
};

#endif /* _SHIPCPANELMULTIFUNCDISPLAYS_H */
