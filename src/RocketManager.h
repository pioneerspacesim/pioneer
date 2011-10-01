#ifndef _ROCKETMANAGER_H
#define _ROCKETMANAGER_H

class RocketSystem;
class RocketRender;

namespace Rocket {
	namespace Core {
		class Context;
	}
}

class RocketManager {
public:
	RocketManager(int width, int height);
	~RocketManager();

	void Draw();

private:
	static bool s_initted;

	int m_width, m_height;

	RocketSystem *m_rocketSystem;
	RocketRender *m_rocketRender;

	Rocket::Core::Context *m_rocketContext;
};

#endif
