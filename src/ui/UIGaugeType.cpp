#include "UIGaugeType.h"
#include "UIGaugeElement.h"

namespace UI {

GaugeType::GaugeType(GaugeElement *el) : m_parent(el)
{
}

GaugeType::~GaugeType()
{
}

Rocket::Core::String GaugeType::GetValue() const
{
	return m_parent->GetAttribute< Rocket::Core::String >("value", "");
}

void GaugeType::OnUpdate()
{
}

void GaugeType::OnRender()
{
}

bool GaugeType::OnAttributeChange(const Rocket::Core::AttributeNameList&)
{
	return true;
}

void GaugeType::OnPropertyChange(const Rocket::Core::PropertyNameList&)
{
}

void GaugeType::OnValueChanged()
{
}

}
