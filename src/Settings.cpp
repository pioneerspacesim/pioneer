// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#include "Settings.h"
#include "KeyBindings.h"
#include "Pi.h"
#if 0
void Display(Settings::KeyMap &key_map)
{
    for (Settings::KeyMap::iterator it = key_map.begin(); it != key_map.end(); ++it)
    {
        for(std::vector<Settings::SVecType >::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            int x = 0;
            for(Settings::SVecType::iterator it3 = it2->begin(); it3 != it2->end(); ++it3)
            {
                std::string th = *it3;
                printf("%s[%d]:  %s\n",it->first.c_str(), x, th.c_str());
                x++;
            }
            // 			printf("%s %s %s %s", it->first.c_str(), it2[0]->c_str(),it2[1].c_str(),it2[2].c_str());
        }
    }
}
#endif
Settings::Settings()
{
    BuildControlBindingList(KeyBindings::BINDING_PROTOS_CONTROLS, m_control_keys,m_control_headers);
    BuildControlBindingList(KeyBindings::BINDING_PROTOS_VIEW, m_view_keys,m_view_headers);

}

Settings::~Settings()
{

}

const Settings::KeyStrings Settings::GetPrettyKeyStrings(const std::string header, const Settings::KeyMap &key_map) const
{
    InnerVector kids;
    for(KeyMap::const_iterator it = key_map.begin(); it != key_map.end(); ++it) {
        if(it->first != header) continue;
        kids = it->second;
        break;
    }
    KeyStrings prettyKeys;;
    for (InnerVector::iterator it = kids.begin(); it != kids.end(); ++it) {
        std::vector<std::string> keys = *it;
        prettyKeys[keys[1]] = keys[3];
    }
    return prettyKeys;
}

void Settings::BuildControlBindingList(const KeyBindings::BindingPrototype *protos,KeyMap &key_map,std::vector<std::string> &header_vec)
{
    assert(protos);

    std::string header;
    for (int i=0; protos[i].label; i++) {
        const KeyBindings::BindingPrototype* proto = &protos[i];


        if (!proto->function) {
            header = proto->label;
            header_vec.push_back(header);
        } else {
            std::vector<std::string> keys;

            std::string func;
            std::string lab;
            std::string bind;
            std::string desc;


            func = proto->function;
            lab = proto->label;
            if (proto->kb) {
                KeyBindings::KeyBinding kb;
                kb = KeyBindings::KeyBindingFromString(Pi::config->String(proto->function));
                bind = KeyBindings::KeyBindingToString(kb);
                desc = kb.Description();


//
            } else if (proto->ab) {
                KeyBindings::AxisBinding ab;
                ab = KeyBindings::AxisBindingFromString(Pi::config->String(func));
                bind = KeyBindings::AxisBindingToString(ab);
                desc = ab.Description();

            } else {
                assert(0);
            }
            keys.push_back(func);
            keys.push_back(lab);
            keys.push_back(bind);
            keys.push_back(desc);

            key_map[header].push_back(keys);
        }
    }

}
