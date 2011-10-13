#ifndef _ROCKETMANAGER_H
#define _ROCKETMANAGER_H

#include "libs.h"
#include <map>

#include "RocketStash.h"

#include "Rocket/Core.h"
#include "Rocket/Controls.h"

class RocketSystem;
class RocketRender;
class RocketEventListenerInstancer;
union SDL_Event;


class RocketEventListener : public Rocket::Core::EventListener {
public:
	RocketEventListener(const std::string &eventName) : Rocket::Core::EventListener(), m_eventName(eventName) {}

	virtual void ProcessEvent(Rocket::Core::Event &e) {
		CallHandler();
	}

	void SetHandler(sigc::slot<void> handler) { m_handler = handler; }
	void CallHandler() {
		if (m_handler) m_handler();
	}

private:
	std::string m_eventName;
	sigc::slot<void> m_handler;
};


class RocketScreen {
public:
	RocketScreen() : m_document(0), m_tooltipActive(false), m_tooltipElement(0) {}
	virtual ~RocketScreen();

	void SetDocument(Rocket::Core::ElementDocument *document);
	Rocket::Core::ElementDocument *GetDocument() const { return m_document; }

	void RegisterKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier, const std::string &eventName);
	void ProcessKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier);

	RocketEventListener *GetEventListener(const std::string &eventName);

	void ShowTooltip(Rocket::Core::Element *sourceElement);
	void ClearTooltip();

private:
	Rocket::Core::ElementDocument *m_document;
	std::map<std::string,RocketEventListener*> m_eventListeners;

	class ShortcutPair {
	public:
		Rocket::Core::Input::KeyIdentifier key;
		Rocket::Core::Input::KeyModifier modifier;

		friend bool operator<(const ShortcutPair &a, const ShortcutPair &b) {
			if (a.key != b.key) return a.key < b.key;
			return a.modifier < b.modifier;
		}
	};
	std::map<ShortcutPair,std::string> m_shortcuts;

	bool m_tooltipActive;
	Rocket::Core::Element *m_tooltipElement;
};


class RocketManager : public Rocket::Core::EventListener, public RocketStash {
public:
	RocketManager(int width, int height);
	~RocketManager();

	RocketScreen *OpenScreen(const std::string &name);
	RocketScreen *GetCurrentScreen() const { return m_currentScreen; }

	virtual void ProcessEvent(Rocket::Core::Event &e);

	void HandleEvent(const SDL_Event *e);

	void Draw();

private:
	int m_width, m_height;

	RocketSystem *m_rocketSystem;
	RocketRender *m_rocketRender;

	RocketEventListenerInstancer *m_rocketEventListenerInstancer;

	Rocket::Core::Context *m_rocketContext;

	std::map<std::string,RocketScreen*> m_screens;
	RocketScreen *m_currentScreen;

	Rocket::Core::Input::KeyIdentifier m_currentKey;
	Rocket::Core::Input::KeyModifier m_currentModifier;

	Uint32 m_tooltipDelayStartTick;
	Rocket::Core::Element *m_tooltipSourceElement;
};

#endif
