#include "Group.h"

namespace Newmodel {

Group::~Group()
{
    for(std::vector<Node*>::iterator itr = m_children.begin();
        itr != m_children.end();
        ++itr)
    {
        (*itr)->DecRefCount();
    }
}

void Group::AddChild(Node *child)
{
	child->IncRefCount();
	m_children.push_back(child);
}

void Group::Render(Graphics::Renderer *r)
{
    for(std::vector<Node*>::iterator itr = m_children.begin();
        itr != m_children.end();
        ++itr)
    {
        (*itr)->Render(r);
    }
}

}