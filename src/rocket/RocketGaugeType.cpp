#include "RocketGaugeType.h"

RocketGaugeType::RocketGaugeType(RocketGaugeElement *el) : parent(el)
{
}

RocketGaugeType::~RocketGaugeType()
{
}

Rocket::Core::String RocketGaugeType::GetValue() const
{
	return element->GetAttribute< Rocket::Core::String >("value", "");
}

void RocketGaugeType::OnUpdate()
{
}

void RocketGaugeType::OnRender()
{
}

bool RocketGaugeType::OnAttributeChange(const Core::AttributeNameList& ROCKET_UNUSED(changed_attributes))
{
	return true;
}

void RocketGaugeType::OnPropertyChange(const Core::PropertyNameList& ROCKET_UNUSED(changed_properties))
{
}

