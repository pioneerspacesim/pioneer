#include "SystemBodyWrapper.h"

#include "AtmosphereParameters.h"
#include "SystemBody.h"
#include "utils.h"

SystemBodyWrapper::SystemBodyWrapper(const SystemBody *sb) :
	m_sbody(sb)
{
	#ifndef NDEBUG
		if (m_sbody == nullptr) {
			Output("SystemBodyWrapper *needs* a non null SystemBody: check your design\n");
			abort();
		}
	#endif // NDEBUG
}

const SystemBody *SystemBodyWrapper::GetSystemBody() const
{
	return m_sbody;
}

GalaxyEnums::BodyType SystemBodyWrapper::GetSystemBodyType() const
{
	return m_sbody->GetType();
}

GalaxyEnums::BodySuperType SystemBodyWrapper::GetSystemBodySuperType() const
{
	return m_sbody->GetSuperType();
}

bool SystemBodyWrapper::IsType(GalaxyEnums::BodyType type) const
{
	return type == m_sbody->GetType();
}

bool SystemBodyWrapper::IsSuperType(GalaxyEnums::BodySuperType type) const
{
	return type == m_sbody->GetSuperType();
}

double SystemBodyWrapper::GetSystemBodyMass() const
{
	return m_sbody->GetMass();
}

double SystemBodyWrapper::GetSystemBodyRadius() const
{
	return m_sbody->GetRadius();
}

const std::string &SystemBodyWrapper::GetSystemBodyName() const
{
	return m_sbody->GetName();
}

const SystemPath &SystemBodyWrapper::GetSystemBodyPath() const
{
	return m_sbody->GetPath();
}

Uint32 SystemBodyWrapper::GetSystemBodySeed() const
{
	return m_sbody->GetSeed();
}

bool SystemBodyWrapper::SystemBodyHasRings() const
{
	return m_sbody->HasRings();
}

const RingStyle &SystemBodyWrapper::GetSystemBodyRings() const
{
	return m_sbody->GetRings();
}

bool SystemBodyWrapper::SystemBodyHasAtmosphere() const
{
	return m_sbody->HasAtmosphere();
}

int SystemBodyWrapper::GetSystemBodyPopulation() const
{
	return m_sbody->GetPopulation();
}

int SystemBodyWrapper::GetSystemBodyAverageTemp() const
{
	return m_sbody->GetAverageTemp();
}

void SystemBodyWrapper::GetSystemBodyAtmosphereFlavor(Color &outColor, double &surfaceDensity) const// kg / m^3
{
	m_sbody->GetAtmosphereFlavor(outColor, surfaceDensity);
}

AtmosphereParameters SystemBodyWrapper::CalcSystemBodyAtmosphereParams() const
{
	return m_sbody->CalcAtmosphereParams();
}
