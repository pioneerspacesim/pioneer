#ifndef _LUACHATFORM_H
#define _LUACHATFORM_H

#include "GenericChatForm.h"
#include "MarketAgent.h"

// only using for including lua headers...
#include "oolua/oolua.h"
#include "oolua/oolua_error.h"

class BBAdvert;
class CommodityTradeWidget;

class LuaChatForm: public GenericChatForm, public MarketAgent {
public:
	void AddOption(const char *text, int val);
	void AddTraderWidget();
	void StartChat(const BBAdvert *);
	void CallDialogHandler(int optionClicked);
	const char *GetStage() const { return m_stage.c_str(); }
	void SetStage(const char *s) { m_stage = s; }
	int GetAdRef() const { return m_modRef; }
	/* MarketAgent stuff */
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t) const;
	bool CanSell(Equip::Type t) const;
	bool DoesSell(Equip::Type t) const;
	int GetStock(Equip::Type t) const;
protected:
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	CommodityTradeWidget *m_commodityTradeWidget;
	void OnClickBuy(int equipType);
	void OnClickSell(int equipType);
	std::string m_stage;
	std::string m_modName;
	int m_modRef;
};

OOLUA_CLASS_NO_BASES(LuaChatForm)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
	OOLUA_MEM_FUNC_0(void, Close)
	OOLUA_MEM_FUNC_0(void, Clear)
	OOLUA_MEM_FUNC_1(void, SetTitle, const char *)
	OOLUA_MEM_FUNC_0(void, AddTraderWidget)
	OOLUA_MEM_FUNC_1(void, SetMessage, const char *)
	OOLUA_MEM_FUNC_2(void, AddOption, const char *, int)
	OOLUA_MEM_FUNC_1(void, SetStage, const char *)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetStage)
	OOLUA_MEM_FUNC_0_CONST(int, GetAdRef)
OOLUA_CLASS_END

#endif /* _LUACHATFORM_H */
