#include "RocketGaugeElement.h"
#include "RocketGaugeTypeBar.h"

RocketGaugeElement::RocketGaugeElement(const Rocket::Core::String &tag) :
	Rocket::Core::Element(tag),
	m_value(1.f)
{
	//default will be a horizontal bar, fill direction right
	m_type = 0;
	m_type = new RocketGaugeTypeBar(this);
	m_typeName = "bar";
	SetClass(m_typeName, true);
}

RocketGaugeElement::~RocketGaugeElement()
{
	delete m_type;
}

Rocket::Core::String RocketGaugeElement::GetName() const
{
	return GetAttribute<Rocket::Core::String>("name", "");
}

void RocketGaugeElement::SetName(const Rocket::Core::String &name)
{
	SetAttribute("name", name);
}

Rocket::Core::String RocketGaugeElement::GetValue() const
{
	return Rocket::Core::String(32, "%f", m_value);
}

void RocketGaugeElement::SetValue(const Rocket::Core::String &value)
{
	SetAttribute("value", value);
	//OnAttributeChange should fire after this I believe
}

float RocketGaugeElement::GetGaugeValue() const
{
	return m_value;
}

void RocketGaugeElement::SetGaugeValue(float val)
{
	m_value = Rocket::Core::Math::Clamp(val, 0.f, 1.f);
}

void RocketGaugeElement::OnUpdate()
{
	m_type->OnUpdate();
}

void RocketGaugeElement::OnRender()
{
	m_type->OnRender();
}

void RocketGaugeElement::OnAttributeChange(const Rocket::Core::AttributeNameList &changedAttributes)
{
  Rocket::Core::Element::OnAttributeChange(changedAttributes);

	if (changedAttributes.find("type") != changedAttributes.end())
	{
		Rocket::Core::String newTypeName = GetAttribute< Rocket::Core::String >("type", "bar");
		if (newTypeName != m_typeName)
		{
			RocketGaugeType *newType = 0;
			
			if (newTypeName == "bar")
				newType = new RocketGaugeTypeBar(this);

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
		m_value = GetAttribute< float >("value", 0.f);
		m_type->OnValueChanged();
	}

	if (!m_type->OnAttributeChange(changedAttributes))
		DirtyLayout();
}

void RocketGaugeElement::OnPropertyChange(const Rocket::Core::PropertyNameList &changedProperties)
{
	Rocket::Core::Element::OnPropertyChange(changedProperties);

	if (m_type != 0)
		m_type->OnPropertyChange(changedProperties);
}

void RocketGaugeElement::ProcessEvent(Rocket::Core::Event &ev)
{
	Rocket::Core::Element::ProcessEvent(ev);
	m_type->ProcessEvent(ev);
}

bool RocketGaugeElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	return m_type->GetIntrinsicDimensions(dimensions);
}

class RocketGaugeElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new RocketGaugeElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void RocketGaugeElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new RocketGaugeElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("gauge", instancer);
	instancer->RemoveReference();
}