// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#ifndef SETTINGS_H
#define SETTINGS_H
#include "KeyBindings.h"
#include "ui/Context.h"
class KeyGetter;
class Settings
{

public:
    Settings();
    virtual ~Settings();
    typedef std::vector <std::vector<std::string> > InnerVector;
    typedef std::map<std::string,InnerVector > KeyMap;
    typedef std::vector<std::string> SVecType;
    typedef std::map<std::string,std::string> MapStrings;

    const SVecType &GetHeaders() const {return m_headers;}
    const KeyMap &GetKeys() const {return m_keys;}
    const std::string GetFunction(const std::string) const;
    const SVecType GetKeysVector(const std::string matcher) const;
    const MapStrings GetPrettyKeyStrings(const std::string header,const KeyMap &key_map);
    
    const SVecType GetVideoModes() const;
    
    const MapStrings GetGameConfig() const;
    bool SaveGameConfig(const Settings::MapStrings ini);
    
    void KeyGrabber(UI::Context *context, std::string matcher, UI::MultiLineText *multi);
    
    void RemoveFWidget();
private:
    enum
    {
        Function, Label, Binding, Key
    };
    
    template <class T>
    T GetNum(std::string &key, const MapStrings &ini) const;
    void BuildControlBindingList(const KeyBindings::BindingPrototype *protos, KeyMap &key_map,SVecType &header_vec);
    void InitControls();
    bool OnChangeKeyBinding(const UI::KeyboardEvent &event, UI::MultiLineText *multi, std::string label);
    
    KeyMap m_control_keys;
    KeyMap m_view_keys;
    SVecType m_control_headers;
    SVecType m_view_headers;
    SVecType m_headers;
    KeyMap m_keys;
    KeyGetter *m_keyGetter;
    std::vector<std::pair<std::string,std::string> > m_updatedKeys;
    sigc::connection m_keyDownConnection;
};

#endif // SETTINGS_H
