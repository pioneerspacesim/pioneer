// no include guard because this file must be included more than once per translation unit

#ifdef Crime_ITEM
Crime_ITEM(TRADING_ILLEGAL_GOODS, (1<<0))
Crime_ITEM(WEAPON_DISCHARGE, (1<<1))
Crime_ITEM(PIRACY, (1<<2))
Crime_ITEM(MURDER, (1<<3))
#undef Crime_ITEM
#endif

#ifdef Bloc_ITEM
Bloc_ITEM(NONE)
Bloc_ITEM(EARTHFED)
Bloc_ITEM(CIS)
Bloc_ITEM(EMPIRE)
#undef Bloc_ITEM
#endif

#ifdef PolitEcon_ITEM
PolitEcon_ITEM(NONE)
PolitEcon_ITEM(VERY_CAPITALIST)
PolitEcon_ITEM(CAPITALIST)
PolitEcon_ITEM(MIXED)
PolitEcon_ITEM(PLANNED)
#undef PolitEcon_ITEM
#endif

#ifdef GovType_ITEM
GovType_ITEM_X(INVALID)

GovType_ITEM(NONE)
GovType_ITEM(EARTHCOLONIAL)
GovType_ITEM(EARTHDEMOC)
GovType_ITEM(EMPIRERULE)
GovType_ITEM(CISLIBDEM)
GovType_ITEM(CISSOCDEM)
GovType_ITEM(LIBDEM)
GovType_ITEM(CORPORATE)
GovType_ITEM(SOCDEM)
GovType_ITEM(EARTHMILDICT)
GovType_ITEM(MILDICT1)
GovType_ITEM(MILDICT2)
GovType_ITEM(EMPIREMILDICT)
GovType_ITEM(COMMUNIST)
GovType_ITEM(PLUTOCRATIC)
GovType_ITEM(DISORDER)

GovType_ITEM_X(MAX)
GovType_ITEM_Y(RAND_MIN, GOV_NONE+1)
GovType_ITEM_Y(RAND_MAX, GOV_MAX-1)
#undef GovType_ITEM
#undef GovType_ITEM_X
#undef GovType_ITEM_Y
#endif

// no include guards (see note at top)
