#ifndef _ROCKETMANAGER_H
#define _ROCKETMANAGER_H

#include <string>
#include <map>

class RocketSystem;
class RocketRender;
union SDL_Event;

namespace Rocket {
	namespace Core {
		class Context;
		class ElementDocument;
	}
}

class RocketManager {
public:
	RocketManager(int width, int height);
	~RocketManager();

	Rocket::Core::ElementDocument *OpenDocument(const std::string &name);
	Rocket::Core::ElementDocument *GetCurrentDocument() { return m_currentDocument; }

	void HandleEvent(const SDL_Event *e);
	void Draw();

private:
	int m_width, m_height;

	RocketSystem *m_rocketSystem;
	RocketRender *m_rocketRender;

	Rocket::Core::Context *m_rocketContext;

	std::map<std::string,Rocket::Core::ElementDocument*> m_documents;
	Rocket::Core::ElementDocument *m_currentDocument;
};

#endif
