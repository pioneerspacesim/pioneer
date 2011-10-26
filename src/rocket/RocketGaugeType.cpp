#include "RocketGaugeType.h"
#include "RocketGaugeElement.h"

RocketGaugeType::RocketGaugeType(RocketGaugeElement *el) : parent(el)
{
}

RocketGaugeType::~RocketGaugeType()
{
}

Rocket::Core::String RocketGaugeType::GetValue() const
{
	return parent->GetAttribute< Rocket::Core::String >("value", "");
}

void RocketGaugeType::OnUpdate()
{
}

void RocketGaugeType::OnRender()
{
}

bool RocketGaugeType::OnAttributeChange(const Rocket::Core::AttributeNameList& ROCKET_UNUSED(changedAttributes))
{
	return true;
}

void RocketGaugeType::OnPropertyChange(const Rocket::Core::PropertyNameList& ROCKET_UNUSED(changedProperties))
{
}

