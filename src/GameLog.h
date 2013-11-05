#ifndef _GAMELOG_H
#define _GAMELOG_H

#include "libs.h"
#include "text/TextureFont.h"
/*
 * For storing all in-game log messages
 * and drawing the last X messages as overlay (all views)
 * atm it holds only 6 messages, but do extend
 */
class GameLog {
public:
	GameLog(RefCountedPtr<Text::TextureFont>, vector2f screenSize);
	void Add(const std::string&);
	void Add(const std::string &from, const std::string &msg);
	void Update(bool paused);
	void DrawHudMessages(Graphics::Renderer*);

private:
	struct Message {
		Message(const std::string &s, Uint32 t);
		std::string msg;
		Uint32 time;
	};
	std::list<Message> m_messages;
	RefCountedPtr<Text::TextureFont> m_font;
	vector2f m_screenSize;
	vector2f m_offset;
	float m_lineHeight;
};

#endif
