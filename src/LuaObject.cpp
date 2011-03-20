#include "LuaObject.h"

static lid next_id = 0;
static std::map<lid, Object*> registry;

LuaObject::LuaObject(Object *o)
{
	m_id = next_id++;
	assert(m_id < (lid)-1);

	Register(o);
}

LuaObject::~LuaObject()
{
	if (m_id == (lid)-1) return;
	Deregister();
}

void LuaObject::Register(Object *o)
{
	registry[m_id] = o;
	o->onDelete.connect(sigc::mem_fun(this, &LuaObject::Deregister));
}

void LuaObject::Deregister()
{
	registry.erase(m_id);
	m_id = -1;
}

Object *LuaObject::Lookup(lid id)
{
	return registry[id];
}
