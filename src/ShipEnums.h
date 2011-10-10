// no include guard because this file must be included more than once per translation unit

#ifdef HyperjumpStatus_ITEM
HyperjumpStatus_ITEM(OK)
HyperjumpStatus_ITEM(CURRENT_SYSTEM)
HyperjumpStatus_ITEM(NO_DRIVE)
HyperjumpStatus_ITEM(OUT_OF_RANGE)
HyperjumpStatus_ITEM(INSUFFICIENT_FUEL)
#undef HyperjumpStatus_ITEM
#endif

#ifdef AlertState_ITEM
AlertState_ITEM(NONE)
AlertState_ITEM(SHIP_NEARBY)
AlertState_ITEM(SHIP_FIRING)
#undef AlertState_ITEM
#endif

// no include guards (see note at top)
