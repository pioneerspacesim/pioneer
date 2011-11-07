#ifndef _UIMANAGER_H
#define _UIMANAGER_H

#include "libs.h"
#include <map>

#include "UIStash.h"

#include "Rocket/Core.h"
#include "Rocket/Controls.h"

union SDL_Event;

namespace UI {

class RocketSystemInterface;
class RocketRenderInterface;
class EventListenerInstancer;


class EventListener : public Rocket::Core::EventListener {
public:
	EventListener(const std::string &eventName) : Rocket::Core::EventListener(), m_eventName(eventName) {}

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


class Screen {
public:
	Screen() : m_document(0), m_tooltipActive(false), m_tooltipElement(0) {}
	virtual ~Screen();

	void SetDocument(Rocket::Core::ElementDocument *document);
	Rocket::Core::ElementDocument *GetDocument() const { return m_document; }

	void RegisterKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier, const std::string &eventName);
	void ProcessKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier);

	EventListener *GetEventListener(const std::string &eventName);

	void ShowTooltip(Rocket::Core::Element *sourceElement);
	void ClearTooltip();

private:
	Rocket::Core::ElementDocument *m_document;
	std::map<std::string,EventListener*> m_eventListeners;

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


class Manager : public Rocket::Core::EventListener, public Stash {
public:
	Manager(int width, int height);
	~Manager();

	template <typename T> void RegisterCustomElement(const std::string &tag) {
		Rocket::Core::ElementInstancer *instancer = new ElementInstancer<T>();
		Rocket::Core::Factory::RegisterElementInstancer(tag.c_str(), instancer);
		instancer->RemoveReference();
	}

	Screen *OpenScreen(const std::string &name);
	Screen *GetCurrentScreen() const { return m_currentScreen; }

	void OpenBackground(const std::string &name);

	virtual void ProcessEvent(Rocket::Core::Event &e);

	void HandleEvent(const SDL_Event *e);

	void Draw();

private:
	template <typename T>
	class ElementInstancer : public Rocket::Core::ElementInstancer {
	    virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
	        return new T(tag);
	    }   

	    virtual void ReleaseElement(Rocket::Core::Element *element) {
	        delete element;
	    }
    
	    virtual void Release() {
	        delete this;
		}
	};

	int m_width, m_height;

	RocketSystemInterface *m_rocketSystemInterface;
	RocketRenderInterface *m_rocketRenderInterface;

	EventListenerInstancer *m_eventListenerInstancer;

	Rocket::Core::Context *m_backgroundContext;
	Rocket::Core::Context *m_mainContext;

	std::map<std::string,Rocket::Core::ElementDocument*> m_backgroundDocuments;
	Rocket::Core::ElementDocument *m_backgroundDocument;

	std::map<std::string,Screen*> m_screens;
	Screen *m_currentScreen;

	Rocket::Core::Input::KeyIdentifier m_currentKey;
	Rocket::Core::Input::KeyModifier m_currentModifier;

	Uint32 m_tooltipDelayStartTick;
	Rocket::Core::Element *m_tooltipSourceElement;
};

}

#endif
