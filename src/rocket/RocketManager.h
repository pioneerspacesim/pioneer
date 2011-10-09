#ifndef _ROCKETMANAGER_H
#define _ROCKETMANAGER_H

#include "libs.h"
#include <map>

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
		if (m_handler) m_handler(&e);
	}

	void SetHandler(sigc::slot<void,Rocket::Core::Event*> handler) { m_handler = handler; }

private:
	std::string m_eventName;
	sigc::slot<void,Rocket::Core::Event*> m_handler;
};


class RocketScreen {
public:
	virtual ~RocketScreen();

	void SetDocument(Rocket::Core::ElementDocument *document);
	Rocket::Core::ElementDocument *GetDocument() const { return m_document; }

	void ProcessKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key);

	RocketEventListener *GetEventListener(const std::string &eventName);

private:
	Rocket::Core::ElementDocument *m_document;
	std::map<std::string,RocketEventListener*> m_eventListeners;
};

class RocketManager : public Rocket::Core::EventListener {
public:
	RocketManager(int width, int height);
	~RocketManager();

	RocketScreen *OpenScreen(const std::string &name);
	RocketScreen *GetCurrentScreen() const { return m_currentScreen; }

	virtual void ProcessEvent(Rocket::Core::Event &e);

	void HandleEvent(const SDL_Event *e);

	void Draw();

	void SetStashItem(const std::string &id, const std::string &value);
	void ClearStashItem(const std::string &id);
	void ClearStash();

private:
	void UpdateScreenFromStash();

	int m_width, m_height;

	RocketSystem *m_rocketSystem;
	RocketRender *m_rocketRender;

	RocketEventListenerInstancer *m_rocketEventListenerInstancer;

	Rocket::Core::Context *m_rocketContext;

	std::map<std::string,RocketScreen*> m_screens;
	RocketScreen *m_currentScreen;

	Rocket::Core::Input::KeyIdentifier m_currentKey;

	std::map<std::string,std::string> m_stash;
	bool m_needsStashUpdate;
};

#endif
