#ifndef _PILUAMODULES_H
#define _PILUAMODULES_H

namespace PiLuaModules {

	void QueueEvent(const char *eventName);
	void QueueEvent(const char *eventName, Object *o1);
	void QueueEvent(const char *eventName, Object *o1, Object *o2);
	void EmitEvents();

	void Init();
	void Uninit();

}

#endif /* _PILUAMODULES_H */
