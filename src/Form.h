// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FORM_H
#define _FORM_H

#include "gui/Gui.h"

class FormController;

class Form : public Gui::Fixed {
public:
	enum FormType { BLANK, FACE };

	virtual FormType GetType() const = 0;

	virtual const std::string &GetTitle() const { return m_title; }
	void SetTitle(const std::string &title) { m_title = title; }

	virtual void OnClose() { }

protected:
	Form(FormController *controller, float w, float h) : Gui::Fixed(w, h), m_formController(controller) {}
	virtual ~Form() {}

	FormController *m_formController;

private:
	std::string m_title;
};


class BlankForm : public Form {
public:
	virtual Form::FormType GetType() const { return Form::BLANK; }

protected:
	BlankForm(FormController *controller) : Form(controller, 780,400) {}
};


class FaceForm : public Form {
public:
	virtual Form::FormType GetType() const { return Form::FACE; }

	virtual Uint32 GetFaceFlags() const { return m_faceFlags; }
	virtual Uint32 GetFaceSeed() const { return m_faceSeed; }
	virtual std::string const& GetCharacterName() const { return m_characterName; }
	virtual std::string const& GetCharacterTitle() const { return m_characterTitle; }

	void SetFaceFlags(Uint32 flags) { m_faceFlags = flags; }
	void SetFaceSeed(Uint32 seed) { m_faceSeed = seed; }
	void SetCharacterName(const std::string& name) { m_characterName = name; }
	void SetCharacterTitle(const std::string& title) { m_characterTitle = title; }

protected:
	FaceForm(FormController *controller) :
		 Form(controller, 470,400), m_faceFlags(0), m_faceSeed(0),
		 m_characterName(""), m_characterTitle("") {}

private:
	Uint32 m_faceFlags;
	Uint32 m_faceSeed;
	std::string m_characterName;
	std::string m_characterTitle;
};


#endif
