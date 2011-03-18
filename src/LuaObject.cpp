#include "LuaObject.h"

static uint32_t next_id = 0;
static std::map<uint32_t, Object*> registry;

LuaObject::LuaObject(Object *o)
{
	m_id = next_id++;
	assert(m_id < (uint32_t)-1);

	Register(o);
}

LuaObject::~LuaObject()
{
	if (m_id == (uint32_t)-1) return;
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

Object *Lookup(uint32_t id)
{
	return registry[id];
}
