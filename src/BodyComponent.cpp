
#include "BodyComponent.h"

size_t BodyComponentDB::m_componentIdx = 0;
std::map<size_t, std::unique_ptr<BodyComponentDB::PoolBase>> BodyComponentDB::m_componentPools;
std::map<std::string, std::unique_ptr<BodyComponentDB::SerializerBase>> BodyComponentDB::m_componentSerializers;
std::map<std::string, BodyComponentDB::PoolBase *> BodyComponentDB::m_componentNames;
std::vector<BodyComponentDB::PoolBase *> BodyComponentDB::m_componentTypes;

std::vector<void (*)()> &GetBodyComponentRegistrars()
{
	static std::vector<void (*)()> m_registrars = {};
	return m_registrars;
}

void BodyComponentDB::Init()
{
	for (auto &registrar : GetBodyComponentRegistrars()) {
		registrar();
	}
}

void BodyComponentDB::Uninit()
{
	// Reset all static variables and free all allocated memory
	m_componentTypes.clear();
	m_componentNames.clear();
	m_componentSerializers.clear();
	m_componentPools.clear();
	m_componentIdx = 0;
}

bool BodyComponentDB::AddComponentRegistrar(void (*registrar)())
{
	GetBodyComponentRegistrars().push_back(registrar);
	return true;
}

BodyComponentDB::PoolBase::~PoolBase()
{
	if (luaInterface)
		delete luaInterface;
}
