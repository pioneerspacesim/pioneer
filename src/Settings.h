// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#ifndef SETTINGS_H
#define SETTINGS_H
#include "KeyBindings.h"
class Settings
{

public:
    Settings();
    virtual ~Settings();
    typedef std::vector <std::vector<std::string> > InnerVector;
    typedef std::map<std::string,InnerVector > KeyMap;
    typedef std::vector<std::string> SVecType;
    typedef std::map<std::string,std::string> KeyStrings;

    const SVecType &GetControlHeaders() const {
        return m_control_headers;
    }
    const KeyMap &GetControlKeys() const {
        return m_control_keys;
    }
    const KeyStrings GetPrettyKeyStrings(const std::string header,const KeyMap &key_map) const;

private:

    void BuildControlBindingList(const KeyBindings::BindingPrototype *protos, KeyMap &key_map,SVecType &header_vec);
    KeyMap m_control_keys;
    KeyMap m_view_keys;
    SVecType m_control_headers;
    SVecType m_view_headers;
};

#endif // SETTINGS_H
