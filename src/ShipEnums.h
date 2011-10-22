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

#ifdef FlightState_ITEM
FlightState_ITEM(FLYING)     // open flight (includes autopilot)
FlightState_ITEM(DOCKING)    // in docking animation
FlightState_ITEM(DOCKED)     // docked with station
FlightState_ITEM(LANDED)     // rough landed (not docked)
FlightState_ITEM(HYPERSPACE) // in hyperspace
#undef FlightState_ITEM
#endif

#ifdef Animation_ITEM
Animation_ITEM(WHEEL_STATE)
#undef Animation_ITEM
#endif

// no include guards (see note at top)
