#include "UIGaugeElement.h"
#include "UIGaugeTypeBar.h"

namespace UI {

GaugeElement::GaugeElement(const Rocket::Core::String &_tag) :
	Rocket::Core::Element(_tag),
	m_value(1.f)
{
	//default will be a horizontal bar, fill direction right
	m_type = 0;
	m_type = new GaugeTypeBar(this);
	m_typeName = "bar";
	SetClass(m_typeName, true);
}

GaugeElement::~GaugeElement()
{
	delete m_type;
}

Rocket::Core::String GaugeElement::GetName() const
{
	return GetAttribute<Rocket::Core::String>("name", "");
}

void GaugeElement::SetName(const Rocket::Core::String &name)
{
	SetAttribute("name", name);
}

Rocket::Core::String GaugeElement::GetValue() const
{
	return Rocket::Core::String(32, "%f", m_value);
}

void GaugeElement::SetValue(const Rocket::Core::String &value)
{
	SetAttribute("value", value);
	//OnAttributeChange should fire after this I believe
}

float GaugeElement::GetGaugeValue() const
{
	return m_value;
}

void GaugeElement::SetGaugeValue(float val)
{
	m_value = Rocket::Core::Math::Clamp(val, 0.f, 1.f);
	m_type->OnValueChanged();
}

void GaugeElement::UpdateFromStash(const float &v)
{
	SetGaugeValue(v);
}

void GaugeElement::OnUpdate()
{
	m_type->OnUpdate();
}

void GaugeElement::OnRender()
{
	m_type->OnRender();
}

void GaugeElement::OnAttributeChange(const Rocket::Core::AttributeNameList &changedAttributes)
{
  Rocket::Core::Element::OnAttributeChange(changedAttributes);

	if (changedAttributes.find("type") != changedAttributes.end())
	{
		Rocket::Core::String newTypeName = GetAttribute< Rocket::Core::String >("type", "bar");
		if (newTypeName != m_typeName)
		{
			GaugeType *newType = 0;
			
			if (newTypeName == "bar")
				newType = new GaugeTypeBar(this);

			if (newType != 0)
			{
				//delete old
				delete m_type;
				m_type = newType;
				SetClass(m_typeName, false);
				SetClass(newTypeName, true);
				m_typeName = newTypeName;

				DirtyLayout();
			}
		}
	}

	if (changedAttributes.find("value") != changedAttributes.end()) {
		SetGaugeValue(GetAttribute< float >("value", 0.f));
	}

	if (!m_type->OnAttributeChange(changedAttributes))
		DirtyLayout();
}

void GaugeElement::OnPropertyChange(const Rocket::Core::PropertyNameList &changedProperties)
{
	Rocket::Core::Element::OnPropertyChange(changedProperties);

	if (m_type != 0)
		m_type->OnPropertyChange(changedProperties);
}

void GaugeElement::ProcessEvent(Rocket::Core::Event &ev)
{
	Rocket::Core::Element::ProcessEvent(ev);
	m_type->ProcessEvent(ev);
}

bool GaugeElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	return m_type->GetIntrinsicDimensions(dimensions);
}

class GaugeElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new GaugeElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void GaugeElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new GaugeElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("gauge", instancer);
	instancer->RemoveReference();
}

}
