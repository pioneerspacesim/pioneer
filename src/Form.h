#ifndef _FORM_H
#define _FORM_H

#include "gui/Gui.h"


class Form : public Gui::Fixed {
public:
	enum FormType { BLANK, FACE };

	virtual FormType GetType() const = 0;

protected:
	Form(float w, float h) : Gui::Fixed(w, h) {}
	virtual ~Form() {}
};


class FaceForm : public Form {
public:
	virtual Form::FormType GetType() const { return Form::FACE; }

	virtual const std::string &GetTitle() const { return m_title; }
	void SetTitle(const std::string &title) { m_title = title; }

	virtual Uint32 GetFaceFlags() const { return m_faceFlags; }
	virtual Uint32 GetFaceSeed() const { return m_faceSeed; }
	
	void SetFaceFlags(int flags) { m_faceFlags = flags; }
	void SetFaceSeed(unsigned int seed) { m_faceSeed = seed; }

protected:
	FaceForm() : Form(470,400), m_faceFlags(0), m_faceSeed(-1UL) {}

private:
	std::string m_title;

	Uint32 m_faceFlags;
	Uint32 m_faceSeed;
};


#endif
