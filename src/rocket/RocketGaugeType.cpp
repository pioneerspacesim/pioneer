#include "RocketGaugeType.h"
#include "RocketGaugeElement.h"

RocketGaugeType::RocketGaugeType(RocketGaugeElement *el) : m_parent(el)
{
}

RocketGaugeType::~RocketGaugeType()
{
}

Rocket::Core::String RocketGaugeType::GetValue() const
{
	return m_parent->GetAttribute< Rocket::Core::String >("value", "");
}

void RocketGaugeType::OnUpdate()
{
}

void RocketGaugeType::OnRender()
{
}

bool RocketGaugeType::OnAttributeChange(const Rocket::Core::AttributeNameList&)
{
	return true;
}

void RocketGaugeType::OnPropertyChange(const Rocket::Core::PropertyNameList&)
{
}

void RocketGaugeType::OnValueChanged()
{
}

