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

class RocketManager {
public:
	RocketManager(int width, int height);
	~RocketManager();

	Rocket::Core::ElementDocument *OpenDocument(const std::string &name);
	Rocket::Core::ElementDocument *GetCurrentDocument() { return m_currentDocument; }

	void RegisterEventHandler(const std::string &eventName, sigc::slot<void,Rocket::Core::Event*> handler);

	void HandleEvent(const SDL_Event *e);
	void Draw();

	void SetStashItem(const std::string &id, const std::string &value);
	void ClearStashItem(const std::string &id);
	void ClearStash();

private:
	void UpdateDocumentFromStash();

	int m_width, m_height;

	RocketSystem *m_rocketSystem;
	RocketRender *m_rocketRender;

	RocketEventListenerInstancer *m_rocketEventListenerInstancer;

	Rocket::Core::Context *m_rocketContext;

	std::map<std::string,Rocket::Core::ElementDocument*> m_documents;
	Rocket::Core::ElementDocument *m_currentDocument;

	std::map<std::string,std::string> m_stash;
	bool m_needsStashUpdate;
};

#endif
