#ifndef _LUACHATFORM_H
#define _LUACHATFORM_H

#include "GenericChatForm.h"

// only using for including lua headers...
#include "oolua/oolua.h"
#include "oolua/oolua_error.h"

class BBAdvert;

class LuaChatForm: public GenericChatForm {
public:
	void AddOption(const char *text, int val);
	void StartChat(const BBAdvert *);
	void CallDialogHandler(int optionClicked);
	int GetStage() const { return m_stage; }
	void SetStage(int s) { m_stage = s; }
private:
	int m_stage;
	std::string m_modName;
	int m_modRef;
};

OOLUA_CLASS_NO_BASES(LuaChatForm)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
	OOLUA_MEM_FUNC_0(void, Close)
	OOLUA_MEM_FUNC_0(void, Clear)
	OOLUA_MEM_FUNC_1(void, SetTitle, const char *)
	OOLUA_MEM_FUNC_1(void, SetMessage, const char *)
	OOLUA_MEM_FUNC_2(void, AddOption, const char *, int)
	OOLUA_MEM_FUNC_1(void, SetStage, int)
	OOLUA_MEM_FUNC_0_CONST(int, GetStage)
OOLUA_CLASS_END

#endif /* _LUACHATFORM_H */
