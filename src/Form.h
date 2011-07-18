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
	
	void SetFaceFlags(Uint32 flags) { m_faceFlags = flags; }
	void SetFaceSeed(Uint32 seed) { m_faceSeed = seed; }

protected:
	FaceForm(FormController *controller) : Form(controller, 470,400), m_faceFlags(0), m_faceSeed(0) {}

private:
	Uint32 m_faceFlags;
	Uint32 m_faceSeed;
};


#endif
