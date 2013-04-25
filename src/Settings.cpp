// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#include "Settings.h"
#include "KeyBindings.h"
#include "Pi.h"
#include "graphics/Graphics.h"
#include "StringF.h"
#include "Lang.h"
#include "Sound.h"
#include "SoundMusic.h"
#include "ui/Context.h"
#include "ui/Button.h"
#include "ui/Label.h"
#include "ui/FloatContainer.h"
#include "ui/Event.h"

#include <sstream>

class KeyGetter {
public:
	KeyGetter(std::string label,std::string matcher, UI::Context *context,std::string fnName) {
		m_function = fnName;
		m_key = matcher.c_str();
		std::string msg = Lang::PRESS_BUTTON_WANTED_FOR + label;
		m_keyLabel = context->Label(msg);
		m_widget = context->Background();
		m_widget->SetInnerWidget(m_keyLabel);
	}
	UI::Background *GetWidget()
	{
		return m_widget;
	}
	UI::Label *GetLabel()
	{
		return m_keyLabel;
	}
	KeyBindings::KeyBinding &GetBinding()
	{
		return m_binding;
	}
	std::pair<std::string,std::string> HandleKeyChange(UI::KeyboardEvent event)
	{

		printf("funcname %s\n", m_function.c_str());
		KeyBindings::KeyBindingFromString(m_key, &m_binding);
		m_binding.type = KeyBindings::KEYBOARD_KEY;
		m_binding.u.keyboard.key = event.keysym.sym;
		// get rid of number lock, caps lock, etc
		m_binding.u.keyboard.mod = SDLMod(event.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_META | KMOD_SHIFT));
		Pi::config->SetString(m_function.c_str(), KeyBindings::KeyBindingToString(m_binding).c_str());
		Pi::config->Save();
		KeyBindings::UpdateBindings();
		std::pair<std::string,std::string> ret  (m_function.c_str(),KeyBindings::KeyBindingToString(m_binding).c_str());
		return ret;
	}
	sigc::signal<void, KeyBindings::KeyBinding> onChange;
private:
	KeyBindings::KeyBinding m_binding;
	UI::Label *m_keyLabel;
	UI::Background *m_widget;
	std::string m_function;
	std::string m_key;
};

Settings::Settings()
{

    InitControls();
    m_keyGetter = 0;
}

Settings::~Settings()
{
	RemoveFWidget();
}

void Settings::InitControls()
{
    m_headers.clear();
    m_keys.clear();
    m_control_keys.clear();
    m_control_headers.clear();
    m_view_keys.clear();
    m_view_headers.clear();
    BuildControlBindingList(KeyBindings::BINDING_PROTOS_CONTROLS, m_control_keys,m_control_headers);
    BuildControlBindingList(KeyBindings::BINDING_PROTOS_VIEW, m_view_keys,m_view_headers);
    m_headers.reserve(m_control_headers.size()+m_view_headers.size());
    m_headers.insert(m_headers.end(),m_control_headers.begin(),m_control_headers.end());
    m_headers.insert(m_headers.end(),m_view_headers.begin(),m_view_headers.end());
    m_keys.insert(m_control_keys.begin(),m_control_keys.end());
    m_keys.insert(m_view_keys.begin(),m_view_keys.end());
}

void Settings::KeyGrabber(UI::Context *context, std::string func, UI::MultiLineText *multi)
{
    Settings::SVecType keys = GetKeysVector(func);
    RemoveFWidget();
    assert(keys.size() > 0);
    assert(m_keyGetter == 0);
    m_keyGetter = new KeyGetter(keys[Label],keys[Key].c_str(), context, func);
    context->AddFloatingWidget(m_keyGetter->GetWidget(),UI::Point(Graphics::GetScreenWidth()/2-200, Graphics::GetScreenHeight()/2), UI::Point(400,40));
    m_keyDownConnection = context->onKeyUp.connect(sigc::bind(sigc::mem_fun(this, &Settings::OnChangeKeyBinding),multi,keys[Key]));
}

void Settings::RemoveFWidget()
{
    if(m_keyGetter != 0)
    {
	    UI::Background *l = m_keyGetter->GetWidget();
	    UI::Context *c = l->GetContext();
	    c->RemoveFloatingWidget(l);
	    delete m_keyGetter;
	    m_keyGetter = 0;
	    m_keyDownConnection.disconnect();
    }
}

const std::string Settings::GetFunction(const std::string matcher) const
{
    return GetKeysVector(matcher)[Function];
}

const Settings::SVecType Settings::GetKeysVector(const std::string matcher) const
{
    for(KeyMap::const_iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        std::string header = it->first;
        InnerVector kids = it->second;
        for (InnerVector::iterator it2 = kids.begin(); it2 != kids.end(); ++it2) {
            std::vector<std::string> keys = *it2;
            if(keys[Function].compare(matcher) == 0 || 
		keys[Label].compare(matcher) == 0 || 
		keys[Binding].compare(matcher) == 0 || 
		keys[Key].compare(matcher) == 0)
            {
                return keys;
            }
        }
    }
    Settings::SVecType failedArray;
    return failedArray;
}

bool Settings::OnChangeKeyBinding(const UI::KeyboardEvent &event, UI::MultiLineText *multi,std::string label)
{
	std::pair<std::string,std::string> updateKeys = m_keyGetter->HandleKeyChange(event);
	m_updatedKeys.push_back(updateKeys);
	multi->SetText(m_keyGetter->GetBinding().Description());
	RemoveFWidget();
	return true;
}

const Settings::MapStrings Settings::GetPrettyKeyStrings(const std::string header, const Settings::KeyMap &key_map)
{
    InitControls();
    InnerVector kids;
    for(KeyMap::const_iterator it = key_map.begin(); it != key_map.end(); ++it) {
        if(it->first != header) continue;
        kids = it->second;
        break;
    }
    MapStrings prettyKeys;;
    for (InnerVector::iterator it = kids.begin(); it != kids.end(); ++it) {
        std::vector<std::string> keys = *it;
        prettyKeys[keys[Label]] = keys[Key];
    }
    return prettyKeys;
}

const Settings::MapStrings Settings::GetGameConfig() const
{
    const std::map<std::string, MapStrings> &map = Pi::config->GetMap();
    MapStrings result;
    for(std::map<std::string, MapStrings>::const_iterator it = map.begin();it != map.end(); ++it) {
	    result.insert(it->second.begin(),it->second.end());
	    printf("%s\n",it->first.c_str());
    }
    return result;
}
bool Settings::SaveGameConfig(const Settings::MapStrings ini)
{
    for(Settings::MapStrings::const_iterator it = ini.begin(); it != ini.end(); ++it){
        Pi::config->SetString(it->first.c_str(), it->second.c_str());
        
    }
    for(std::vector<std::pair<std::string, std::string> >::const_iterator it = m_updatedKeys.begin(); it != m_updatedKeys.end(); ++it)
    {
	    Pi::config->SetString(it->first.c_str(), it->second.c_str());
    }
    m_updatedKeys.clear();
    
    Pi::config->Save();
//     InitControls();
    
    std::string t = "DetailPlanets";
    Pi::detail.planets = GetNum<int>(t, ini);
    
    t = "Textures";
    Pi::detail.textures = GetNum<int>(t, ini);
    
    t = "FractalMultiple";
    Pi::detail.fracmult = GetNum<int>(t,ini);
    
    t= "DetailCities";
    Pi::detail.cities = GetNum<int>(t,ini);
    
    t = "InvertMouseY";
    Pi::SetMouseYInvert(GetNum<int>(t,ini));
    
    t = "DisplayNavTunnel";
    Pi::SetNavTunnelDisplayed(GetNum<int>(t,ini));
    
    t = "EnableJoystick";
    Pi::SetJoystickEnabled(GetNum<bool>(t,ini));
    
    t = "MasterMuted";
    int iMute = GetNum<int>(t,ini);
    Sound::Pause(iMute);
    if(iMute == 0)
    {
	t = "MasterVolume";
	Sound::SetMasterVolume(GetNum<float>(t,ini));
    }
    
    t = "MusicMuted";
    bool bMute = GetNum<bool>(t,ini);
    Pi::GetMusicPlayer().SetEnabled(bMute);
    if(!bMute)
    {
	t = "MusicVolume";
	Pi::GetMusicPlayer().SetVolume(GetNum<float>(t,ini));
    }
    
    t = "SfxMuted";
    if(GetNum<int>(t,ini) == 1)
	Sound::SetSfxVolume(0.f);
    else
    {
	t = "SfxVolume";
	Sound::SetSfxVolume(GetNum<float>(t,ini));
    }
	    
    
    Pi::OnChangeDetailLevel();
    return true;
}

template <class T>
T Settings::GetNum(std::string &key, const MapStrings &ini) const
{
    
    MapStrings::const_iterator it;
    it = ini.find(key);
    std::istringstream buffer(it->second);
    T numb;
    buffer >> numb;
    return numb;
}

const Settings::SVecType Settings::GetVideoModes() const
{
    SVecType result;
    const std::vector<Graphics::VideoMode> m_videoModes = Graphics::GetAvailableVideoModes();

    for (std::vector<Graphics::VideoMode>::const_iterator it = m_videoModes.begin(); it != m_videoModes.end(); ++it) {
        std::string tmp = stringf(Lang::X_BY_X, formatarg("x", int(it->width)), formatarg("y", int(it->height)));
        result.push_back(tmp);
    }
    return result;
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

            std::string func(proto->function);
            std::string lab;
            std::string bind;
            std::string key_string;//will probably need to be a vector for multiple assignments


//             func = proto->function;
            lab = proto->label;
            if (proto->kb) {
                KeyBindings::KeyBinding kb;
                kb = KeyBindings::KeyBindingFromString(Pi::config->String(proto->function));
                bind = KeyBindings::KeyBindingToString(kb);
                key_string = kb.Description();


//
            } else if (proto->ab) {
                KeyBindings::AxisBinding ab;
                ab = KeyBindings::AxisBindingFromString(Pi::config->String(func));
                bind = KeyBindings::AxisBindingToString(ab);
                key_string = ab.Description();

            } else {
                assert(0);
            }
            keys.push_back(func);
            keys.push_back(lab);
            keys.push_back(bind);
            keys.push_back(key_string);

            key_map[header].push_back(keys);
        }
    }

}
