#ifndef PI_LANGUAGE
    #define PI_LANGUAGE en
#endif

#define PiLang PiLanguages::PI_LANGUAGE

#ifdef PI_LANG_ONCE
    #define PL(a,b) const char* a = b
#else
    #define PL(a,b) extern const char* a
#endif

/* PL(hi_there,"Hi, there!");
 * 
 * can be picked up as a translated
 * string by using the following:
 * 
 * PiLang::hi_there
 */
namespace PiLanguages {
    namespace en {
        // General
        PL(SUGGESTED_RESPONSES,
                "Suggested responses:");
        PL(CASH,
                "Cash");
        PL(LEGAL_STATUS,
                "Legal status");
        PL(CARGO_SPACE,
                "Cargo space");
        PL(ITEM,
                "Item");
        PL(SHIP,
                "Ship");
        PL(PRICE,
                "Price");
        PL(BUY,
                "Buy");
        PL(SELL,
                "Sell");
        PL(STOCK,
                "Stock");
        PL(CARGO,
                "Cargo");
        PL(VID_LINK_DOWN,
                "Video link down");
        PL(VID_LINK_ESTABLISHED,
                "Video link established");
        PL(VID_CONNECTING,
                "Connecting...");
        PL(BOUGHT_1T_OF,
                "You have bought 1t of %s.");
        PL(SOLD_1T_OF,
                "You have sold 1t of %s.");
        PL(WELCOME_TO_MARKET,
                "Welcome to %s commodities market");
        PL(GO_BACK,
                "Go back");
        PL(FITTING,
                "Fitting ");  // Preserve trailing space
        PL(REMOVING,
                "Removing "); // Preserve trailing space
        PL(FIT_TO_WHICH_MOUNT,
                "Fit laser to which gun mount?");
        PL(REMOVE_FROM_WHICH_MOUNT,
                "Remove laser from which gun mount?");
        PL(YOU_NOT_ENOUGH_MONEY,
                "You do not have enough money");
        PL(TRADER_NOT_ENOUGH_MONEY,
                "Trader does not have enough money");
        PL(NO_SPACE_ON_SHIP,
                "There is no space on your ship");
        PL(SOMEWHERE_SERVICES,
                "%s services");
        PL(SOMEWHERE_SHIPYARD,
                "%s shipyard");
        PL(SOMEWHERE_SHIP_REPAIRS,
                "%s ship repairs");
        PL(PRICE_TO_FIT,
                "$ to fit");
        PL(PRICE_TO_REMOVE,
                "$ for removal");
        PL(WT,
                "Wt");
        PL(FIT,
                "Fit");
        PL(REMOVE,
                "Remove");
        PL(BUY_THIS_SHIP,
                "Buy this ship");
        PL(SHIP_TYPE,
                "Ship type");
        PL(REGISTRATION_ID,
                "Registration id");
        PL(WEIGHT_EMPTY,
                "Weight empty");
        PL(NUMBER_TONNES,
                "%dt");
        PL(WEIGHT_FULLY_LADEN,
                "Weight fully loaded");
        PL(CAPACITY,
                "Capacity");
        PL(FORWARD_ACCEL_EMPTY,
                "Forward accel (empty)");
        PL(NUMBER_G,
                "%.1f G");
        PL(FORWARD_ACCEL_LADEN,
                "Forward accel (laden)");
        PL(REVERSE_ACCEL_EMPTY,
                "Reverse accel (empty)");
        PL(REVERSE_ACCEL_LADEN,
                "Reverse accel (laden)");
        PL(HYPERDRIVE_FITTED,
                "Hyperdrive fitted:");
        PL(HYPERSPACE_RANGE_LADEN,
                "Hyperspace range (fully laden):");
        PL(THANKS_AND_REMEMBER_TO_BUY_FUEL,
                "Thank you for your purchase. Remember to fit equipment and buy fuel before you depart.");
        PL(CLASS_NUMBER,
                "Class %d");
        PL(NUMBER_LY,
                "%.2f ly");
        PL(SHIP_IS_ALREADY_FULLY_REPAIRED,
                "Your ship is in perfect working condition.");
        PL(REPAIR_1_PERCENT_HULL,
                "Repair 1.0% of hull damage");
        PL(REPAIR_ENTIRE_HULL,
                "Repair all hull damage (%.1f%%)");
        PL(REPAIR,
                "Repair");
        PL(PART_EX,
                "Part exchange");
        PL(VIEW,
                "View");
        PL(SHIP_EQUIPMENT,
                "Ship equipment");
        PL(SOMEWHERE_SHIP_EQUIPMENT,
                "%s ship equipment");
        PL(REPAIRS_AND_SERVICING,
                "Repairs and servicing");
        PL(NEW_AND_RECONDITIONED_SHIPS,
                "New and reconditioned ships");
        PL(BULLETIN_BOARD,
                "Bulletin Board");
        PL(WELCOME_TO_SOMEWHERE,
                "Welcome to %s");
        PL(SPACESTATION_LONG_WELCOME_MESSAGE,
                "Hello, traveller.  Welcome to this space station.  We have reports of vandalism and graffiti on board.  If you see that any of our signage is misleading, mis-spelled or just missing, please inform a member of staff.\n\nOxygen is currently free of charge.");
        PL(REQUEST_LAUNCH,
                "Request Launch");
        PL(SHIPYARD,
                "Shipyard");
        PL(COMMODITIES_MARKET,
                "Commodities market");
        PL(SOMEWHERE_COMMODITIES_MARKET,
                "%s commodities market");
        PL(SOMEWHERE_SHIP_MARKET,
                "%s ship market");
        PL(CONTACT_LOCAL_POLICE,
                "Contact local police");
        PL(COMMS_LINK,
                "Comms link");
        PL(ZOOM_IN,
                "Zoom in");
        PL(ZOOM_OUT,
                "Zoom out");
        PL(NORMA_ARM,
                "Norma arm");
        PL(PERSEUS_ARM,
                "Perseus arm");
        PL(OUTER_ARM,
                "Outer arm");
        PL(SAGITTARIUS_ARM,
                "Sagittarius arm");
        PL(SCUTUM_CENTAURUS_ARM,
                "Scutum-Centaurus arm");
        PL(INT_LY,
                "%d ly");

        // Config / game control
        PL(PRESS_BUTTON_WANTED_FOR,
                "Press the button you want for "); // Preserve trailing space
        PL(MOVE_AXIS_WANTED_FOR,
                "Move the joystick axis you want for "); // Preserve trailing space
        PL(SAVE,
                "Save");
        PL(LOAD,
                "Load");
        PL(CANCEL,
                "Cancel");
        PL(SELECT_FILENAME_TO_SAVE,
                "Select a file to save to or enter a new filename");
        PL(GAME_SAVED_TO,
                "Game saved to "); // Preserve trailing space
        PL(SELECT_FILENAME_TO_LOAD,
                "Select a file to load");
        PL(COULD_NOT_OPEN_FILENAME,
                "Could not open %s");
        PL(GAME_LOAD_CORRUPT,
                "This saved game cannot be loaded because it contains errors.");
        PL(GAME_LOAD_CANNOT_OPEN,
                "This saved game file could not be opened due to permissions or something...");
        PL(LOW,
                "Low");
        PL(MEDIUM,
                "Medium");
        PL(HIGH,
                "High");
        PL(VERY_HIGH,
                "Very high");
        PL(VERY_VERY_HIGH,
                "Very very high");
        PL(SIGHTS_SOUNDS_SAVES,
                "Sights, sounds & saving games");
        PL(PIONEER,
                "PIONEER");
        PL(SAVE_THE_GAME,
                "[S] Save the game");
        PL(LOAD_A_GAME,
                "[L] Load a game");
        PL(EXIT_THIS_GAME,
                "Exit this game");
        PL(WINDOW_OR_FULLSCREEN,
                "Windowed or fullscreen (restart to apply)");
        PL(FULL_SCREEN,
                "Full screen");
        PL(OTHER_GRAPHICS_SETTINGS,
                "Other graphics settings");
        PL(USE_SHADERS,
                "Use shaders");
        PL(USE_HDR,
                "Use HDR Lighting (looks cool)");
        PL(SOUND_SETTINGS,
                "Sound settings");
        PL(VOL_MASTER,
                "Master:");
        PL(VOL_EFFECTS,
                "Effects:");
        PL(VOL_MUSIC,
                "Music:");
        PL(VIDEO_RESOLUTION,
                "Video resolution (restart game to apply)");
        PL(X_BY_X,
                "%dx%d");
        PL(PLANET_DETAIL_LEVEL,
                "Planet detail level:");
        PL(CITY_DETAIL_LEVEL,
                "City detail level:");
        PL(CONTROLS,
                "Controls");
        PL(ENABLE_JOYSTICK,
                "Enable joystick control");
        PL(MOUSE_INPUT,
                "Mouse Input");
        PL(INVERT_MOUSE_Y,
                "Invert MouseY");
 
        // Wares
        PL(NONE,
                "None");
        PL(HYDROGEN,
                "Hydrogen");
        PL(HYDROGEN_DESCRIPTION,
                "Hydrogen is primarily used as a fusion fuel");
        PL(LIQUID_OXYGEN,
                "Liquid Oxygen");
        PL(LIQUID_OXYGEN_DESCRIPTION,
                "Oxygen is required for life support systems and some industrial processes");
        PL(METAL_ORE,
                "Metal ore");
        PL(CARBON_ORE,
                "Carbon ore");
        PL(CARBON_ORE_DESCRIPTION,
                "Carbon ores (coal and oil) are required for the synthesis of many useful chemicals, including plastics, synthetic foodstuffs, medicines and textiles");
        PL(METAL_ALLOYS,
                "Metal alloys");
        PL(PLASTICS,
                "Plastics");
        PL(FRUIT_AND_VEG,
                "Fruit and Veg");
        PL(ANIMAL_MEAT,
                "Animal Meat");
        PL(LIVE_ANIMALS,
                "Live Animals");
        PL(LIQUOR,
                "Liquor");
        PL(GRAIN,
                "Grain");
        PL(TEXTILES,
                "Textiles");
        PL(FERTILIZER,
                "Fertilizer");
        PL(WATER,
                "Water");
        PL(MEDICINES,
                "Medicines");
        PL(CONSUMER_GOODS,
                "Consumer goods");
        PL(COMPUTERS,
                "Computers");
        PL(ROBOTS,
                "Robots");
        PL(PRECIOUS_METALS,
                "Precious metals");
        PL(INDUSTRIAL_MACHINERY,
                "Industrial machinery");
        PL(FARM_MACHINERY,
                "Farm machinery");
        PL(MINING_MACHINERY,
                "Mining machinery");
        PL(AIR_PROCESSORS,
                "Air processors");
        PL(SLAVES,
                "Slaves");
        PL(HAND_WEAPONS,
                "Hand weapons");
        PL(BATTLE_WEAPONS,
                "Battle weapons");
        PL(NERVE_GAS,
                "Nerve Gas");
        PL(NARCOTICS,
                "Narcotics");
        PL(MILITARY_FUEL,
                "Military fuel");
        PL(RUBBISH,
                "Rubbish");
        PL(RADIOACTIVES,
                "Radioactive waste");
 
        // Hardware
        PL(MISSILE_UNGUIDED,
                "R40 Unguided Rocket");
        PL(MISSILE_GUIDED,
                "Guided Missile");                                                     
        PL(MISSILE_SMART,
                "Smart Missile");                                                   
        PL(MISSILE_NAVAL,
                "Naval Missile");
        PL(ATMOSPHERIC_SHIELDING,
                "Atmospheric Shielding");
        PL(ATMOSPHERIC_SHIELDING_DESCRIPTION,
                "Shields your spaceship from the heat of atmospheric re-entry.");
        PL(ECM_BASIC,
                "ECM system");
        PL(ECM_BASIC_DESCRIPTION,
                "An electronic countermeasure missile defence system, capable of destroying some homing missiles.");
        PL(SCANNER,
                "Scanner");
        PL(SCANNER_DESCRIPTION,
                "Provides a 3D map of nearby ships.");
        PL(ECM_ADVANCED,
                "Advanced ECM system");
        PL(ECM_ADVANCED_DESCRIPTION,
                "An electronic countermeasure missile defence system, capable of destroying more advanced types of homing missiles.");
        PL(SHIELD_GENERATOR,
                "Shield Generator");
        PL(SHIELD_GENERATOR_DESCRIPTION,
                "Provides additional hull defences with each unit fitted.");
        PL(LASER_COOLING_BOOSTER,
                "Laser Cooling Booster");
        PL(LASER_COOLING_BOOSTER_DESCRIPTION,
                "An improved cooling system for your weapons.");
        PL(CARGO_LIFE_SUPPORT,
                "Cargo Bay Life Support");
        PL(CARGO_LIFE_SUPPORT_DESCRIPTION,
                "Allows the transport of live cargo.");
        PL(AUTOPILOT,
                "Autopilot");
        PL(AUTOPILOT_DESCRIPTION,
                "An on-board flight computer.");
        PL(RADAR_MAPPER,
                "Radar Mapper");
        PL(RADAR_MAPPER_DESCRIPTION,
                "Used to remotely inspect the equipment, cargo and state of other ships.");
        PL(FUEL_SCOOP,
                "Fuel Scoop");
        PL(FUEL_SCOOP_DESCRIPTION,
                "Permits scooping hydrogen fuel from gas giant planets.");
        PL(HYPERCLOUD_ANALYZER,
                "Hypercloud Analyzer");
        PL(HYPERCLOUD_ANALYZER_DESCRIPTION,
                "Analyze hyperspace clouds to determine destination and time of arrival or departure.");
        PL(HULL_AUTOREPAIR,
                "Hull Auto-Repair System");
        PL(HULL_AUTOREPAIR_DESCRIPTION,
                "Automatically repairs the ship's hull in the event of damage.");
        PL(SHIELD_ENERGY_BOOSTER,
                "Shield Energy Booster");
        PL(SHIELD_ENERGY_BOOSTER_DESCRIPTION,
                "Increases the rate at which shields recharge.");
        PL(DRIVE_CLASS1,
                "Class 1 Hyperdrive");
        PL(DRIVE_CLASS2,
                "Class 2 Hyperdrive");
        PL(DRIVE_CLASS3,
                "Class 3 Hyperdrive");
        PL(DRIVE_CLASS4,
                "Class 4 Hyperdrive");
        PL(DRIVE_CLASS5,
                "Class 5 Hyperdrive");
        PL(DRIVE_CLASS6,
                "Class 6 Hyperdrive");
        PL(DRIVE_CLASS7,
                "Class 7 Hyperdrive");
        PL(DRIVE_CLASS8,
                "Class 8 Hyperdrive");
        PL(DRIVE_CLASS9,
                "Class 9 Hyperdrive");
        PL(DRIVE_MIL1,
                "Class 1 Military drive");
        PL(DRIVE_MIL2,
                "Class 2 Military drive");
        PL(DRIVE_MIL3,
                "Class 3 Military drive");
        PL(DRIVE_MIL4,
                "Class 4 Military drive");
        PL(PULSECANNON_1MW,
                "1MW pulse cannon");
        PL(PULSECANNON_DUAL_1MW,
                "1MW dual-fire pulse cannon");
        PL(PULSECANNON_2MW,
                "2MW pulse cannon");
        PL(PULSECANNON_RAPID_2MW,
                "2MW rapid-fire pulse cannon");
        PL(PULSECANNON_4MW,
                "4MW pulse cannon");
        PL(PULSECANNON_10MW,
                "10MW pulse cannon");
        PL(PULSECANNON_20MW,
                "20MW pulse cannon");
        PL(MININGCANNON_17MW,
                "17MW blast-mining cannon");
        PL(MININGCANNON_17MW_DESCRIPTION,
                "Used to blast-mine mineral rich asteroids.");
        PL(SMALL_PLASMA_ACCEL,
                "Small plasma accelerator");
        PL(LARGE_PLASMA_ACCEL,
                "Large plasma accelerator");
        PL(CLEAN,
                "Clean");
        PL(HYPERSPACE_ARRIVAL_CLOUD,
                "Hyperspace arrival cloud");
        PL(HYPERSPACE_DEPARTURE_CLOUD,
                "Hyperspace departure cloud");
        PL(TYPE,
                "Type");
        PL(CLIENT,
                "Client");
        PL(LOCATION,
                "Location");
        PL(DUE,
                "Due");
        PL(REWARD,
                "Reward");
        PL(STATUS,
                "Status");
        PL(CARGO_INVENTORY,
                "Cargo Inventory:");
        PL(JETTISON,
                "Jettison");
        PL(JETTISONED,
                "Jettisoned 1 tonne of "); // preserve trailing space
        PL(COMBAT_RATING,
                "COMBAT RATING:");
        PL(CRIMINAL_RECORD,
                "CRIMINAL RECORD:");
        PL(SHIP_INFORMATION_HEADER,
                "SHIP INFORMATION:  "); // preserve trailing space
        PL(HYPERDRIVE,
                "Hyperdrive");
        PL(FREE,
                "Free");
        PL(USED,
                "Used");
        PL(TOTAL_WEIGHT,
                "All-up weight");
        PL(FRONT_WEAPON,
                "Front weapon");
        PL(REAR_WEAPON,
                "Rear weapon");
        PL(HYPERSPACE_RANGE,
                "Hyperspace range");
        PL(NO_MOUNTING,
                "no mounting");
        PL(SHIP_INFORMATION,
                "Ship information");
        PL(REPUTATION,
                "Reputation");
        PL(MISSIONS,
                "Missions");
        PL(SHIFT,
                "shift "); // preserve trailing space
        PL(CTRL,
                "ctrl "); // preserve trailing space
        PL(ALT,
                "alt "); // preserve trailing space
        PL(META,
                "meta "); // preserve trailing space
        PL(JOY,
                "Joy");
        PL(BUTTON,
                " Button "); // preserve leading and trailing space
        PL(HAT,
                " Hat"); // preserve leading space
        PL(DIRECTION,
                " Dir "); // preserve leading and trailing space
        PL(X,
                "X");
        PL(Y,
                "Y");
        PL(Z,
                "Z");
        PL(AXIS,
                " Axis"); // preserve leading space
        PL(WEAPONS,
                "Weapons");
        PL(TARGET_OBJECT_IN_SIGHTS,
                "Target object in cross-hairs");
        PL(FIRE_LASER,
                "Fire laser");
        PL(SHIP_ORIENTATION,
                "Ship orientation");
        PL(FAST_ROTATION_CONTROL,
                "Fast rotational control");
        PL(PITCH_UP,
                "Pitch up");
        PL(PITCH_DOWN,
                "Pitch down");
        PL(YAW_LEFT,
                "Yaw left");
        PL(YAW_RIGHT,
                "Yaw right");
        PL(ROLL_LEFT,
                "Roll left");
        PL(ROLL_RIGHT,
                "Roll right");
        PL(MANUAL_CONTROL_MODE,
                "Manual control mode");
        PL(THRUSTER_MAIN,
                "Thrust forward");
        PL(THRUSTER_RETRO,
                "Thrust backwards");
        PL(THRUSTER_VENTRAL,
                "Thrust up");
        PL(THRUSTER_DORSAL,
                "Thrust down");
        PL(THRUSTER_PORT,
                "Thrust left");
        PL(THRUSTER_STARBOARD,
                "Thrust right");
        PL(SPEED_CONTROL_MODE,
                "Speed control mode");
        PL(INCREASE_SET_SPEED,
                "Increase set speed");
        PL(DECREASE_SET_SPEED,
                "Decrease set speed");
        PL(JOYSTICK_INPUT,
                "Joystick input");
        PL(PITCH,
                "Pitch");
        PL(ROLL,
                "Roll");
        PL(YAW,
                "Yaw");
        PL(MISSILE,
                "missile");
        PL(HARMLESS,
                "Harmless");
        PL(MOSTLY_HARMLESS,
                "Mostly harmless");
        PL(POOR,
                "Poor");
        PL(AVERAGE,
                "Average");
        PL(ABOVE_AVERAGE ,
                "Above Average");
        PL(COMPETENT,
                "Competent");
        PL(DANGEROUS,
                "Dangerous");
        PL(DEADLY,
                "Deadly");
        PL(ELITE,
                "ELITE");
        PL(SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS,
                "Simulating evolution of the universe: %.1f billion years ;-)");
        PL(TOMBSTONE_EPITAPH,
                "RIP OLD BEAN");
        PL(MM_START_NEW_GAME_EARTH,
                "New game starting on Earth");
        PL(MM_START_NEW_GAME_E_ERIDANI,
                "New game starting on Epsilon Eridani");
        PL(MM_START_NEW_GAME_DEBUG,
                "New game starting on debug point");
        PL(MM_LOAD_SAVED_GAME,
                "Load a saved game");
        PL(MM_QUIT,
                "Quit");
        PL(SCREENSHOT_FILENAME_TEMPLATE,
                "screenshot%08d.png");
        PL(PIONEERING_PILOTS_GUILD,
                "Pioneering Pilots' Guild");
        PL(RIGHT_ON_COMMANDER,
                "Well done commander! Your combat rating has improved!");
        PL(ALERT_CANCELLED,
                "Alert cancelled.");
        PL(SHIP_DETECTED_NEARBY,
                "Ship detected nearby.");
        PL(DOWNGRADING_ALERT_STATUS,
                "No fire detected for 60 seconds, downgrading alert status.");
        PL(LASER_FIRE_DETECTED,
                "Laser fire detected.");
        PL(SOMEWHERE_POLICE,
                "%s Police");
        PL(WE_HAVE_NO_BUSINESS_WITH_YOU,
                "We have no business with you at the moment.");
        PL(YOU_MUST_PAY_FINE_OF_N_CREDITS,
                "We do not tolerate crime. You must pay a fine of %s.");
        PL(PAY_THE_FINE_NOW,
                "Pay the fine now.");
        PL(HANG_UP,
                "Hang up.");
        PL(TRADING_ILLEGAL_GOODS,
                "Trading illegal goods");
        PL(UNLAWFUL_WEAPONS_DISCHARGE,
                "Unlawful weapons discharge");
        PL(PIRACY,
                "Piracy");
        PL(MURDER,
                "Murder");
        PL(INDEPENDENT,
                "Independent");
        PL(EARTH_FEDERATION,
                "Earth Federation");
        PL(INDEPENDENT_CONFEDERATION,
                "Confederation of Independent Systems");
        PL(EMPIRE,
                "The Empire");
        PL(NO_ESTABLISHED_ORDER,
                "No established order");
        PL(HARD_CAPITALIST,
                "Entirely Capitalist - no government welfare provision");
        PL(CAPITALIST,
                "Capitalist");
        PL(MIXED_ECONOMY,
                "Mixed economy");
        PL(PLANNED_ECONOMY,
                "Centrally planned economy");
        PL(NO_CENTRAL_GOVERNANCE,
                "No central governance");
        PL(EARTH_FEDERATION_COLONIAL_RULE,
                "Earth Federation Colonial Rule");
        PL(EARTH_FEDERATION_DEMOCRACY,
                "Earth Federation Democracy");
        PL(IMPERIAL_RULE,
                "Imperial Rule");
        PL(LIBERAL_DEMOCRACY,
                "Liberal democracy");
        PL(SOCIAL_DEMOCRACY,
                "Social democracy");
        PL(CORPORATE_SYSTEM,
                "Corporate system");
        PL(MILITARY_DICTATORSHIP,
                "Military dictatorship");
        PL(COMMUNIST,
                "Communist");
        PL(PLUTOCRATIC_DICTATORSHIP,
                "Plutocratic dictatorship");
        PL(VIOLENT_ANARCHY,
                "Disorder - Overall governance contested by armed factions");
        PL(X_CANNOT_BE_TOLERATED_HERE,
                "%s cannot be tolerated here.");
        PL(SECTOR_X_Y,
                "Sector: %d,%d");
        PL(DISTANCE_FUEL_TIME,
                "Dist. %.2f light years (fuel required: %dt | time loss: %.1fhrs)");
        PL(CURRENT_SYSTEM,
                "Current system");
        PL(DISTANCE_FUEL_REQUIRED,
                "Dist. %.2f light years (insufficient fuel, required: %dt)");
        PL(DISTANCE_OUT_OF_RANGE,
                "Dist. %.2f light years (out of range)");
        PL(CANNOT_HYPERJUMP_WITHOUT_WORKING_DRIVE,
                "You cannot perform a hyperjump because you do not have a functioning hyperdrive");
        PL(QUADRUPLE_SYSTEM,
                "Quadruple system. "); // preserve trailing space
        PL(TRIPLE_SYSTEM,
                "Triple system. "); // preserve trailing space
        PL(BINARY_SYSTEM,
                "Binary system. "); // preserve trailing space
        PL(FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED,
                "Fuel scoop active. You now have %d tonnes of hydrogen.");
        PL(CARGO_BAY_LIFE_SUPPORT_LOST,
                "Sensors report critical cargo bay life-support conditions.");
        PL(NO_FREE_SPACE_FOR_ITEM,
                "You have no free space for this item.");
        PL(SHIP_IS_FULLY_LADEN,
                "Your ship is fully laden.");
        PL(YOU_DO_NOT_HAVE_ANY_X,
                "You do not have any %s.");
        PL(REAR_VIEW,
                "Rear view");
        PL(EXTERNAL_VIEW,
                "External view");
        PL(NAVIGATION_STAR_MAPS,
                "Navigation and star maps");
        PL(COMMS,
                "Comms");
        PL(GALAXY_SECTOR_VIEW,
                "Galaxy sector view");
        PL(SYSTEM_ORBIT_VIEW,
                "System orbit view");
        PL(STAR_SYSTEM_INFORMATION,
                "Star system information");
        PL(GALACTIC_VIEW,
                "Galactic view");
        PL(NO_ALERT,
                "No alert");
        PL(SHIP_NEARBY,
                "Ship nearby");
        PL(FIRE_DETECTED,
                "Fire detected");
        PL(DOCKING_CLEARANCE_EXPIRED,
                "Docking clearance expired. If you wish to dock you must repeat your request.");
        PL(MESSAGE_FROM_X,
                "Message from %s:");
        PL(SELECT_A_TARGET,
                "Select a target");
        PL(FRONT,
                "Front");
        PL(REAR,
                "Rear");
        PL(POLICE_SHIP_REGISTRATION,
                "POLICE");
        PL(CLEARANCE_ALREADY_GRANTED_BAY_N,
                "Clearance already granted. Proceed to docking bay %d.");
        PL(CLEARANCE_GRANTED_BAY_N,
                "Clearance granted. Proceed to docking bay %d.");
        PL(CLEARANCE_DENIED_NO_BAYS,
                "Clearance denied. There are no free docking bays.");
        PL(ITEM_IS_OUT_OF_STOCK,
                "This item is out of stock.");
        PL(BROWN_DWARF,
                "Brown dwarf sub-stellar object");
        PL(WHITE_DWARF,
                "White dwarf stellar remnant");
        PL(STAR_M,
                "Type 'M' red star");
        PL(STAR_K,
                "Type 'K' orange star");
        PL(STAR_G,
                "Type 'G' yellow star");
        PL(STAR_F,
                "Type 'F' white star");
        PL(STAR_A,
                "Type 'A' hot white star");
        PL(STAR_B,
                "Bright type 'B' blue star");
        PL(STAR_O,
                "Hot, massive type 'O' star");
        PL(STAR_M_GIANT,
                "Red giant star");
        PL(STAR_K_GIANT,
                "Orange giant star - Unstable");
        PL(STAR_G_GIANT,
                "Yellow giant star - Unstable");
        PL(STAR_AF_GIANT,
                "White giant star");
        PL(STAR_B_GIANT,
                "Blue giant star");
        PL(STAR_O_GIANT,
                "Hot Blue giant star");
        PL(STAR_M_SUPER_GIANT,
                "Red super giant star");
        PL(STAR_K_SUPER_GIANT,
                "Orange super giant star");
        PL(STAR_G_SUPER_GIANT,
                "Yellow super giant star");
        PL(STAR_AF_SUPER_GIANT,
                "White super giant star");
        PL(STAR_B_SUPER_GIANT,
                "Blue super giant star");
        PL(STAR_O_SUPER_GIANT,
                "Hot Blue super giant star");
        PL(STAR_M_HYPER_GIANT,
                "Red hyper giant star");
        PL(STAR_K_HYPER_GIANT,
                "Orange hyper giant star - Unstable");
        PL(STAR_G_HYPER_GIANT,
                "Yellow hyper giant star - Unstable");
        PL(STAR_AF_HYPER_GIANT,
                "White hyper giant star");
        PL(STAR_B_HYPER_GIANT,
                "Blue hyper giant star");
        PL(STAR_O_HYPER_GIANT,
                "Hot Blue hyper giant star");
        PL(STAR_M_WF,
                "Wolf-Rayet star - Unstable");
        PL(STAR_B_WF,
                "Wolf-Rayet star - Risk of collapse");
        PL(STAR_O_WF,
                "Wolf-Rayet star - Imminent collapse");
        PL(STAR_S_BH,
                "A stellar black hole");
        PL(STAR_IM_BH,
                "An intermediate-mass black hole");
        PL(STAR_SM_BH,
                "Our galactic anchor");
        PL(VERY_LARGE_GAS_GIANT,
                "Very large gas giant");
        PL(LARGE_GAS_GIANT,
                "Large gas giant");
        PL(MEDIUM_GAS_GIANT,
                "Medium gas giant");
        PL(SMALL_GAS_GIANT,
                "Small gas giant");
        PL(ASTEROID,
                "Asteroid");
        PL(MASSIVE,
                "Massive");
        PL(LARGE,
                "Large");
        PL(TINY,
                "Tiny");
        PL(SMALL,
                "Small");
        PL(COMMA_HIGHLY_VOLCANIC,
                ", highly volcanic");
        PL(HIGHLY_VOLCANIC,
                "Highly volcanic");
        PL(ICE_WORLD,
                " ice world"); // preserve leading space
        PL(ROCKY_PLANET,
                " rocky planet"); // preserve leading space
        PL(OCEANICWORLD,
                " oceanic world"); // preserve leading space
        PL(PLANET_CONTAINING_LIQUID_WATER,
                " planet containing liquid water"); // preserve leading space
        PL(PLANET_WITH_SOME_ICE,
                " planet with some ice"); // preserve leading space
        PL(ROCKY_PLANET_CONTAINING_COME_LIQUIDS,
                " rocky planet containing some liquids,"); // preserve leading space
        PL(WITH_NO_SIGNIFICANT_ATMOSPHERE,
                " with no significant atmosphere"); // preserve leading space
        PL(TENUOUS,
                "tenuous");
        PL(THIN,
                "thin");
        PL(THICK,
                "thick");
        PL(VERY_DENSE,
                "very dense");
        PL(WITH_A,
                " with a "); // preserve leading and trailing space
        PL(O2_ATMOSPHERE,
                " Oxygen atmosphere"); // preserve leading space
        PL(CO2_ATMOSPHERE,
                " Carbon Dioxide atmosphere"); // preserve leading space
        PL(CO_ATMOSPHERE,
                " Carbon Monoxide atmosphere"); // preserve leading space
        PL(CH4_ATMOSPHERE,
                " Methane atmosphere"); // preserve leading space
        PL(H_ATMOSPHERE,
                " Hydrogen atmosphere"); // preserve leading space
        PL(HE_ATMOSPHERE,
                " Helium atmosphere"); // preserve leading space
        PL(AR_ATMOSPHERE,
                " Argon atmosphere"); // preserve leading space
        PL(S_ATMOSPHERE,
                " Sulfuric atmosphere"); // preserve leading space
        PL(N_ATMOSPHERE,
                " Nitrogen atmosphere"); // preserve leading space
        PL(AND_HIGHLY_COMPLEX_ECOSYSTEM,
                " and a highly complex ecosystem."); // preserve leading space
        PL(AND_INDIGENOUS_PLANT_LIFE,
                " and indigenous plant life."); // preserve leading space
        PL(AND_INDIGENOUS_MICROBIAL_LIFE,
                " and indigenous microbial life."); // preserve leading space
        PL(ORBITAL_STARPORT,
                "Orbital starport");
        PL(STARPORT,
                "Starport");
        PL(UNKNOWN,
                "<unknown>");
        PL(UNEXPLORED_SYSTEM_NO_DATA,
                "Unexplored system. No more data available.");
        PL(SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS,
                "Small-scale prospecting. No registered settlements.");
        PL(SMALL_INDUSTRIAL_OUTPOST,
                "Small industrial outpost.");
        PL(SOME_ESTABLISHED_MINING,
                "Some established mining.");
        PL(YOUNG_FARMING_COLONY,
                "Young farming colony.");
        PL(INDUSTRIAL_COLONY,
                "Industrial colony.");
        PL(MINING_COLONY,
                "Mining colony.");
        PL(OUTDOOR_AGRICULTURAL_WORLD,
                "Outdoor agricultural world.");
        PL(HEAVY_INDUSTRY,
                "Heavy industry.");
        PL(EXTENSIVE_MINING,
                "Extensive mining operations.");
        PL(THRIVING_OUTDOOR_WORLD,
                "Thriving outdoor world.");
        PL(INDUSTRIAL_HUB_SYSTEM,
                "Industrial hub system.");
        PL(VAST_STRIP_MINE,
                "Vast strip-mining colony.");
        PL(HIGH_POPULATION_OUTDOOR_WORLD,
                "High population outdoor world.");
        PL(SOMEWHERE_SPACEPORT,
                " Spaceport"); // preserve leading space
        PL(SOMEWHERE_STARPORT,
                " Starport"); // preserve leading space
        PL(MASS,
                "Mass");
        PL(N_WHATEVER_MASSES,
                "%.3f %s masses");
        PL(SOLAR,
                "Solar");
        PL(EARTH,
                "Earth");
        PL(SURFACE_TEMPERATURE,
                "Surface temperature");
        PL(N_CELSIUS,
                "%d C");
        PL(N_YEARS,
                "%.1f years");
        PL(N_DAYS,
                "%.1f days");
        PL(ORBITAL_PERIOD,
                "Orbital period");
        PL(PERIAPSIS_DISTANCE,
                "Periapsis distance");
        PL(APOAPSIS_DISTANCE,
                "Apoapsis distance");
        PL(ECCENTRICITY,
                "Eccentricity");
        PL(AXIAL_TILE,
                "Axial tilt");
        PL(N_DEGREES,
                "%.1f degrees");
        PL(DAY_LENGTH,
                "Day length");
        PL(N_EARTH_DAYS,
                "%.1f earth days");
        PL(STARPORTS,
                "Starports");
        PL(MAJOR_IMPORTS,
                "Major Imports:");
        PL(MINOR_IMPORTS,
                "Minor Imports:");
        PL(MAJOR_EXPORTS,
                "Major Exports:");
        PL(MINOR_EXPORTS,
                "Minor Exports:");
        PL(ILLEGAL_GOODS,
                "Illegal Goods:");
        PL(UNEXPLORED_SYSTEM_STAR_INFO_ONLY,
                "Unexplored System. Star information has been gathered by remote telescope, but no planetary information is available.");
        PL(PLANETARY_INFO,
                "Planetary info");
        PL(ECONOMIC_INFO,
                "Economic info");
        PL(DEMOGRAPHICS,
                "Demographics");
        PL(STABLE_SYSTEM_WITH_N_MAJOR_BODIES_STARPORTS,
                "Stable system with %d major %s and %d starport%s.\n\n%s");
        PL(BODY,
                "body");
        PL(BODIES,
                "bodies");
        PL(PLURAL_SUFFIX,
                "s");
        PL(SYSTEM_TYPE,
                "System type:");
        PL(GOVERNMENT_TYPE,
                "Government type:");
        PL(ECONOMY_TYPE,
                "Economy type:");
        PL(ALLEGIANCE,
                "Allegiance:");
        PL(POPULATION,
                "Population:");
        PL(OVER_N_BILLION,
                "Over %d billion");
        PL(OVER_N_MILLION,
                "Over %d million");
        PL(A_FEW_THOUSAND,
                "Only a few thousand");
        PL(NO_REGISTERED_INHABITANTS,
                "No registered inhabitants");
        PL(SECTOR_COORDINATES,
                "Sector coordinates:");
        PL(SYSTEM_NUMBER,
                "System number:");
        PL(NAME,
                "Name");
        PL(ROTATIONAL_PERIOD,
                " (rotational period)"); // preserve leading space
        PL(RADIUS,
                "Radius");
        PL(SEMI_MAJOR_AXIS,
                "Semi-major axis");
        PL(TIME_POINT,
                "Time point: ");
        PL(UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW,
                "Unexplored system. System view unavailable.");
        PL(WHEELS_ARE_UP,
                "Wheels are up");
        PL(WHEELS_ARE_DOWN,
                "Wheels are down");
        PL(OBJECT_LABELS_ARE_ON,
                "Object labels are on");
        PL(OBJECT_LABELS_ARE_OFF,
                "Object labels are off");
        PL(HYPERSPACE_JUMP,
                "Hyperspace Jump");
        PL(TAKEOFF,
                "Takeoff");
        PL(MANUAL_CONTROL,
                "Manual control");
        PL(COMPUTER_SPEED_CONTROL,
                "Computer speed control");
        PL(AUTOPILOT_ON,
                "Autopilot on");
        PL(SHIP_VELOCITY_BY_REFERENCE_OBJECT,
                "Ship velocity by reference object");
        PL(DISTANCE_FROM_SHIP_TO_NAV_TARGET,
                "Distance from ship to navigation target");
        PL(SHIP_ALTITUDE_ABOVE_TERRAIN,
                "Ship altitude above terrain");
        PL(EXTERNAL_ATMOSPHERIC_PRESSURE,
                "External atmospheric pressure");
        PL(HULL_TEMP,
                "Hull temp");
        PL(WEAPON_TEMP,
                "Weapon temp");
        PL(HULL_INTEGRITY,
                "Hull integrity");
        PL(SHIELD_INTEGRITY,
                "Shield integrity");
        PL(LAUNCH_PERMISSION_DENIED_BUSY,
                "Permission to launch denied: docking bay busy.");
        PL(HYPERSPACE_JUMP_ABORTED,
                "Hyperspace jump aborted.");
        PL(CAMERA,
                "camera");
        PL(LANDED,
                "Landed");
        PL(DOCKING,
                "Docking");
        PL(DOCKED,
                "Docked");
        PL(HYPERSPACE,
                "Hyperspace");
        PL(SET_SPEED_KM_S,
                "Set speed: %.2f km/s");
        PL(SET_SPEED_M_S,
                "Set speed: %.0f m/s");
        PL(KM_S_RELATIVE_TO,
                "%.2f km/s rel-to %s");
        PL(M_S_RELATIVE_TO,
                "%.0f m/s rel-to %s");
        PL(N_DISTANCE_TO_TARGET,
                "%s to target");
        PL(ALT_IN_METRES,
                "Alt: %.0fm");
        PL(PRESSURE_N_BAR,
                "P: %.2f bar");
        PL(NO_HYPERDRIVE,
                "No hyperdrive");
        PL(MASS_N_TONNES,
                "Mass: %dt");
        PL(SHIELD_STRENGTH_N,
                "Shield strength: %.2f");
        PL(CARGO_N,
                "Cargo: %dt");
        PL(HYPERSPACE_ARRIVAL_CLOUD_REMNANT,
                "Hyperspace arrival cloud remnant");
        PL(HYPERSPACE_X_CLOUD,
                "Hyperspace %s cloud");
        PL(SHIP_MASS_N_TONNES,
                "Ship mass: %dt");
        PL(DATE_DUE_N,
                "Date due: %s");
        PL(ARRIVAL,
                "arrival");
        PL(DEPARTURE,
                "departure");
        PL(SOURCE,
                "Origin");
        PL(DESTINATION,
                "Destination");
        PL(HYPERSPACING_IN_N_SECONDS,
                "Hyperspacing in %.0f seconds");
        PL(NAVIGATION_TARGETS_IN_THIS_SYSTEM,
                "Navigation targets in this system");
        PL(YOU_NO_MONEY,
                "You do not have any money.");
        PL(FINE_PAID_N_BUT_N_REMAINING,
                "You have paid %s but still have an outstanding fine of %s.");
        PL(FINE_PAID_N,
                "You have paid the fine of %s.");
        PL(SET_HYPERSPACE_DESTINATION_TO,
                "Set hyperspace destination to "); // preserve trailing space
        PL(NO_TARGET_SELECTED,
                "Ship Computer: No target selected");
        PL(REQUEST_DOCKING_CLEARANCE,
                "Request docking clearance");
        PL(AUTOPILOT_DOCK_WITH_STATION,
                "Autopilot: Dock with space station");
        PL(PAY_FINE_REMOTELY,
                "Pay fine by remote transfer (%s)");
        PL(AUTOPILOT_FLY_TO_VICINITY_OF,
                "Autopilot: Fly to vicinity of "); // preserve trailing space
        PL(AUTOPILOT_ENTER_LOW_ORBIT_AROUND,
                "Autopilot: Enter low orbit around "); // preserve trailing space
        PL(AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND,
                "Autopilot: Enter medium orbit around "); // preserve trailing space
        PL(AUTOPILOT_ENTER_HIGH_ORBIT_AROUND,
                "Autopilot: Enter high orbit around "); // preserve trailing space
        PL(SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE,
                "Hyperspace cloud analyzer: Set hyperspace target to follow this departure");
        PL(SET_HYPERSPACE_TARGET_TO,
                "Set hyperspace target to "); // preserve trailing space
    }
    namespace de {
        // General
        PL(SUGGESTED_RESPONSES,
                "vorgeschlagen Antworten:");
        PL(CASH,
                "Geld");
        PL(LEGAL_STATUS,
                "Rechtlichen Status");
        PL(CARGO_SPACE,
                "Laderaum");
        PL(ITEM,
                "Artikel");
        PL(SHIP,
                "Shiff");
        PL(PRICE,
                "Pries");
        PL(BUY,
                "Kaufen");
        PL(SELL,
                "Verkaufen");
        PL(STOCK,
                "Auf Lager");
        PL(CARGO,
                "Ladung");
        PL(VID_LINK_DOWN,
                "Video-Link getrennt");
        PL(VID_LINK_ESTABLISHED,
                "Video-Link verbunden");
        PL(VID_CONNECTING,
                "Verbinden...");
        PL(BOUGHT_1T_OF,
                "Sie haben 1t %s gekauft.");
        PL(SOLD_1T_OF,
                "Sie haben 1t %s verkauft.");
        PL(WELCOME_TO_MARKET,
                "Willkommen auf dem Rohstoffmarkt von %s");
        PL(GO_BACK,
                "Zurückgehen");
        PL(FITTING,
                "Installieren: ");  // Preserve trailing space
        PL(REMOVING,
                "Entfernen: "); // Preserve trailing space
        PL(FIT_TO_WHICH_MOUNT,
                "Welche Position zu diesem Laserkanone zu installieren?");
        PL(REMOVE_FROM_WHICH_MOUNT,
                "Welche Position zu diesem Laserkanone aus zu entfernen?");
        PL(YOU_NOT_ENOUGH_MONEY,
                "Sie haben nicht genug Geld");
        PL(TRADER_NOT_ENOUGH_MONEY,
                "Der Händler hat nicht genug Geld");
        PL(NO_SPACE_ON_SHIP,
                "Es gibt keinen Raum in Ihrem Schiff");
        PL(SOMEWHERE_SERVICES,
                "%s Dienst");
        PL(SOMEWHERE_SHIPYARD,
                "%s Werft");
        PL(SOMEWHERE_SHIP_REPAIRS,
                "%s Reparaturdienst");
        PL(PRICE_TO_FIT,
                "$ zu installieren");
        PL(PRICE_TO_REMOVE,
                "$ zu entfernen");
        PL(WT,
                "Gw");
        PL(FIT,
                "Inst.");
        PL(REMOVE,
                "Entf.");
        PL(BUY_THIS_SHIP,
                "Diese Raumschiff kaufen");
        PL(SHIP_TYPE,
                "Schiff Art");
        PL(REGISTRATION_ID,
                "Registrierungs-ID");
        PL(WEIGHT_EMPTY,
                "Leergewicht");
        PL(NUMBER_TONNES,
                "%dt");
        PL(WEIGHT_FULLY_LADEN,
                "Gesamtgewicht");
        PL(CAPACITY,
                "Hubraum");
        PL(FORWARD_ACCEL_EMPTY,
                "Vorwärts-Beschleunigung (unbeladen)");
        PL(NUMBER_G,
                "%.1f G");
        PL(FORWARD_ACCEL_LADEN,
                "Vorwärts-Beschleunigung (beladen)");
        PL(REVERSE_ACCEL_EMPTY,
                "Rückwärts-Beschleunigung (unbeladen)");
        PL(REVERSE_ACCEL_LADEN,
                "Rückwärts-Beschleunigung (beladen)");
        PL(HYPERDRIVE_FITTED,
                "Hyperlaufwerk installiert:");
        PL(HYPERSPACE_RANGE_LADEN,
                "Fahrbereich im Hyperraum (voll beladen):");
        PL(THANKS_AND_REMEMBER_TO_BUY_FUEL,
                "Vielen Dank für Ihren Einkauf. Denken Sie daran, Schiffsausrüstung und Kraftstoff zu kaufen, bevor Sie abdocken.");
        PL(CLASS_NUMBER,
                "Klasse %d");
        PL(NUMBER_LY,
                "%.2f Lj");
        PL(SHIP_IS_ALREADY_FULLY_REPAIRED,
                "Ihr Schiff ist ganz in Ordnung.");
        PL(REPAIR_1_PERCENT_HULL,
                "1.0% den Rumpf reparieren");
        PL(REPAIR_ENTIRE_HULL,
                "Den ganzen Rumpf reparieren (%.1f%%)");
        PL(REPAIR,
                "Reparieren");
        PL(PART_EX,
                "Teil Börsenpreis");
        PL(VIEW,
                "Ansehen");
        PL(SHIP_EQUIPMENT,
                "Schiffsausrüstung");
        PL(SOMEWHERE_SHIP_EQUIPMENT,
                "%s Schiffsausrüstung");
        PL(REPAIRS_AND_SERVICING,
                "Reparaturen und Service");
        PL(NEW_AND_RECONDITIONED_SHIPS,
                "Neue und überholte Raumschiffe");
        PL(BULLETIN_BOARD,
                "Nachrichten und Missionen");
        PL(WELCOME_TO_SOMEWHERE,
                "Wilkommen bei %s");
        PL(SPACESTATION_LONG_WELCOME_MESSAGE,
                "Wilkommen am bord.  Heute ist das Luft ganz kostenlos.  Bitte nutzen Sie unsere zahlreichen Dienstleistungen.");
        PL(REQUEST_LAUNCH,
                "Ansuchen Erlaubnis um zu abdocken");
        PL(SHIPYARD,
                "Werft");
        PL(COMMODITIES_MARKET,
                "Rohstoffmarkt");
        PL(SOMEWHERE_COMMODITIES_MARKET,
                "%s Rohstoffmarkt");
        PL(SOMEWHERE_SHIP_MARKET,
                "%s Raumschiffmarkt");
        PL(CONTACT_LOCAL_POLICE,
                "Um den örtliche Polizei");
        PL(COMMS_LINK,
                "Kommunikationsverbindung");
        PL(ZOOM_IN,
                "Vergrößern");
        PL(ZOOM_OUT,
                "Verkleinern");
        PL(NORMA_ARM,
                "Norma-arm");
        PL(PERSEUS_ARM,
                "Perseus-arm");
        PL(OUTER_ARM,
                "Cygnus-arm");
        PL(SAGITTARIUS_ARM,
                "Sagittarius-arm");
        PL(SCUTUM_CENTAURUS_ARM,
                "Scutum-Crux-arm");
        PL(INT_LY,
                "%d Lj");

        // Config / game control
        PL(PRESS_BUTTON_WANTED_FOR,
                "Tastedrücken, die Sie wollen um zuweisen nach "); // Preserve trailing space
        PL(MOVE_AXIS_WANTED_FOR,
                "Joystick Achsebewegen, die Sie wollen um zuweisen nach "); // Preserve trailing space
        PL(SAVE,
                "Speichern");
        PL(LOAD,
                "Laden");
        PL(CANCEL,
                "Abbrechen");
        PL(SELECT_FILENAME_TO_SAVE,
                "Wählen Sie den Namen der Datei");
        PL(GAME_SAVED_TO,
                "Spiel gespeichert im "); // Preserve trailing space
        PL(SELECT_FILENAME_TO_LOAD,
                "Wählen Sie eine Datei zu laden");
        PL(COULD_NOT_OPEN_FILENAME,
                "%s konnte nicht geöffnet");
        PL(GAME_LOAD_CORRUPT,
                "Konnte nicht Spiel geladen werden, weil die Datei beschädigt wurde.");
        PL(GAME_LOAD_CANNOT_OPEN,
                "Die Spieldatei konnte nicht geöffnet werden.");
        PL(LOW,
                "Niedrig");
        PL(MEDIUM,
                "Mittel");
        PL(HIGH,
                "Hohe");
        PL(VERY_HIGH,
                "Sehr hohe");
        PL(VERY_VERY_HIGH,
                "Extrem hohe");
        PL(SIGHTS_SOUNDS_SAVES,
                "Anblicke, Geräusche, Speichern von Spielen");
        PL(PIONEER,
                "PIONEER");
        PL(SAVE_THE_GAME,
                "[S] Spiel speichern");
        PL(LOAD_A_GAME,
                "[L] Spiel laden");
        PL(EXIT_THIS_GAME,
                "Spiel beenden");
        PL(WINDOW_OR_FULLSCREEN,
                "Fenstermodus oder im Vollbildmodus  (Programm neu gestartet werden)");
        PL(FULL_SCREEN,
                "Vollbildmodus");
        PL(OTHER_GRAPHICS_SETTINGS,
                "Andere Grafikeinstellungen");
        PL(USE_SHADERS,
                "Pixel-Shader aktivieren");
        PL(USE_HDR,
                "HDR-Beleuchtung aktivieren");
        PL(SOUND_SETTINGS,
                "Sound-Einstellungen");
        PL(VOL_MASTER,
                "Master:");
        PL(VOL_EFFECTS,
                "Effekte:");
        PL(VOL_MUSIC,
                "Musik:");
        PL(VIDEO_RESOLUTION,
                "Video-Auflösung (Programm muss neu gestartet werden)");
        PL(X_BY_X,
                "%dx%d");
        PL(PLANET_DETAIL_LEVEL,
                "Planet Detailebene:");
        PL(CITY_DETAIL_LEVEL,
                "Stadt Detailebene:");
        PL(CONTROLS,
                "Steuerung");
        PL(ENABLE_JOYSTICK,
                "Joystick-Steuerung ermöglichen");
        PL(MOUSE_INPUT,
                "Mauseingabe");
        PL(INVERT_MOUSE_Y,
                "Maus Y-Achse invertieren");
 
        // Wares
        PL(NONE,
                "Nichts");
        PL(HYDROGEN,
                "Wasserstoff");
        PL(HYDROGEN_DESCRIPTION,
                "Wasserstoff ist meistens als eine Fusion Brennstoff verwendet");
        PL(LIQUID_OXYGEN,
                "Flüssigen Sauerstoff");
        PL(LIQUID_OXYGEN_DESCRIPTION,
                "Sauerstoff ist für die lebenserhaltenden Systeme und einige industrielle Prozesse benötigt");
        PL(METAL_ORE,
                "Metallerz");
        PL(CARBON_ORE,
                "Carbon-Erz");
        PL(CARBON_ORE_DESCRIPTION,
                "Carbon-Erze (Kohle und Öl) sind für die Synthese von vielen nützlichen Chemikalien, darunter Kunststoffe, synthetische Lebensmittel, Arzneimittel und Textilien erforderlich");
        PL(METAL_ALLOYS,
                "Legierungen");
        PL(PLASTICS,
                "Kunststoffe");
        PL(FRUIT_AND_VEG,
                "Obst und Gemüse");
        PL(ANIMAL_MEAT,
                "Tierfleisch");
        PL(LIVE_ANIMALS,
                "Lebenden Tieren");
        PL(LIQUOR,
                "Alkoholische Getränke");
        PL(GRAIN,
                "Getreide");
        PL(TEXTILES,
                "Textilien");
        PL(FERTILIZER,
                "Düngemittel");
        PL(WATER,
                "Wasser");
        PL(MEDICINES,
                "Arzneimittel");
        PL(CONSUMER_GOODS,
                "Consumer goods");
        PL(COMPUTERS,
                "Computer");
        PL(ROBOTS,
                "Roboter");
        PL(PRECIOUS_METALS,
                "Edelmetalle");
        PL(INDUSTRIAL_MACHINERY,
                "Industriemaschinen");
        PL(FARM_MACHINERY,
                "Landmaschinen");
        PL(MINING_MACHINERY,
                "Bergbaumaschinen");
        PL(AIR_PROCESSORS,
                "Luft-Prozessoren");
        PL(SLAVES,
                "Sklaven");
        PL(HAND_WEAPONS,
                "Handwaffen");
        PL(BATTLE_WEAPONS,
                "Kriegwaffen");
        PL(NERVE_GAS,
                "Nervengift");
        PL(NARCOTICS,
                "Narkotika");
        PL(MILITARY_FUEL,
                "Militär Kraftstoff");
        PL(RUBBISH,
                "Abfallstoffe");
        PL(RADIOACTIVES,
                "Radioaktiver Abfälle");
 
        // Hardware
        PL(MISSILE_UNGUIDED,
                "R40 Ungelenkte Rakete");
        PL(MISSILE_GUIDED,
                "Geführte Rakete");
        PL(MISSILE_SMART,
                "Smart-Rakete");
        PL(MISSILE_NAVAL,
                "Marine-Rakete");
        PL(ATMOSPHERIC_SHIELDING,
                "Atmosphärischen Hitzeshild");
        PL(ATMOSPHERIC_SHIELDING_DESCRIPTION,
                "Schützt Ihr Raumschiff von der Hitze der Eintritt in die Atmosphäre.");
        PL(ECM_BASIC,
                "EloGM-System");
        PL(ECM_BASIC_DESCRIPTION,
                "Eine elektronische Gegenmaßnahme Raketenabwehrsystem, zerstören können einige geführte Raketen.");
        PL(SCANNER,
                "Scanner");
        PL(SCANNER_DESCRIPTION,
                "Bietet eine 3D-Karte von Schiffen in der Nähe.");
        PL(ECM_ADVANCED,
                "Fortgeschritten EloGM-System");
        PL(ECM_ADVANCED_DESCRIPTION,
                "Eine elektronische Gegenmaßnahme Raketenabwehrsystem, zerstören können fortgeschrittene Arten von geführte Raketen.");
        PL(SHIELD_GENERATOR,
                "Schild-Generator");
        PL(SHIELD_GENERATOR_DESCRIPTION,
                "Bietet zusätzlichen Schutz Rumpf mit jedem Gerät eingebaut.");
        PL(LASER_COOLING_BOOSTER,
                "Laserkühlung Booster");
        PL(LASER_COOLING_BOOSTER_DESCRIPTION,
                "Eine verbesserte Waffen Kühlsystem.");
        PL(CARGO_LIFE_SUPPORT,
                "Leben Unterstützung für Frachtraum");
        PL(CARGO_LIFE_SUPPORT_DESCRIPTION,
                "Ermöglicht den Transport von lebenden Fracht.");
        PL(AUTOPILOT,
                "Autopilot");
        PL(AUTOPILOT_DESCRIPTION,
                "Ein an-bord Flugcomputer.");
        PL(RADAR_MAPPER,
                "Radar Kartierungsmaschine");
        PL(RADAR_MAPPER_DESCRIPTION,
                "Aus der Ferne überprüfen die Ausrüstung, Fracht und den Zustand der anderen Schiffe.");
        PL(FUEL_SCOOP,
                "Kraftstoffschaufel");
        PL(FUEL_SCOOP_DESCRIPTION,
                "Erlaubt Schöpfen Wasserstoff aus Gasriesen..");
        PL(HYPERCLOUD_ANALYZER,
                "Hyperraumwolke Analysator");
        PL(HYPERCLOUD_ANALYZER_DESCRIPTION,
                "Analysieren Hyperraumwolken zum Ziel und Zeitpunkt der Ankunft oder Abreise zu bestimmen.");
        PL(HULL_AUTOREPAIR,
                "System der Rumpf selbst-reparieren");
        PL(HULL_AUTOREPAIR_DESCRIPTION,
                "Repariert automatisch den Schiffsrumpf im Falle eines Schadens.");
        PL(SHIELD_ENERGY_BOOSTER,
                "Schild Energie Booster");
        PL(SHIELD_ENERGY_BOOSTER_DESCRIPTION,
                "Erhöht die Wiederaufladerrate der Schilde.");
        PL(DRIVE_CLASS1,
                "Klasse 1 Hyperlaufwerk");
        PL(DRIVE_CLASS2,
                "Klasse 2 Hyperlaufwerk");
        PL(DRIVE_CLASS3,
                "Klasse 3 Hyperlaufwerk");
        PL(DRIVE_CLASS4,
                "Klasse 4 Hyperlaufwerk");
        PL(DRIVE_CLASS5,
                "Klasse 5 Hyperlaufwerk");
        PL(DRIVE_CLASS6,
                "Klasse 6 Hyperlaufwerk");
        PL(DRIVE_CLASS7,
                "Klasse 7 Hyperlaufwerk");
        PL(DRIVE_CLASS8,
                "Klasse 8 Hyperlaufwerk");
        PL(DRIVE_CLASS9,
                "Klasse 9 Hyperlaufwerk");
        PL(DRIVE_MIL1,
                "Klasse 1 Militärlaufwerk");
        PL(DRIVE_MIL2,
                "Klasse 2 Militärlaufwerk");
        PL(DRIVE_MIL3,
                "Klasse 3 Militärlaufwerk");
        PL(DRIVE_MIL4,
                "Klasse 4 Militärlaufwerk");
        PL(PULSECANNON_1MW,
                "1MW Pulskanone");
        PL(PULSECANNON_DUAL_1MW,
                "1MW doppelläufigen Pulskanone");
        PL(PULSECANNON_2MW,
                "2MW Pulskanone");
        PL(PULSECANNON_RAPID_2MW,
                "2MW doppelläufigen Pulskanone");
        PL(PULSECANNON_4MW,
                "4MW Pulskanone");
        PL(PULSECANNON_10MW,
                "10MW Pulskanone");
        PL(PULSECANNON_20MW,
                "20MW Pulskanone");
        PL(MININGCANNON_17MW,
                "17Mg Bergbaukanone");
        PL(MININGCANNON_17MW_DESCRIPTION,
                "Dient zum Hochofen-Mine mineralreichen Asteroiden.");
        PL(SMALL_PLASMA_ACCEL,
                "Kleines Plasmawerfer");
        PL(LARGE_PLASMA_ACCEL,
                "Großes Plasmawerfer");
        PL(CLEAN,
                "Sauber");
        PL(HYPERSPACE_ARRIVAL_CLOUD,
                "Hyperraum Anflug Wolke");
        PL(HYPERSPACE_DEPARTURE_CLOUD,
                "Hyperraum Abflug Wolke");
        PL(TYPE,
                "Typ");
        PL(CLIENT,
                "Auftraggeber");
        PL(LOCATION,
                "Ansiedlung");
        PL(DUE,
                "Fälligkeit");
        PL(REWARD,
                "Entgelt");
        PL(STATUS,
                "Status");
        PL(CARGO_INVENTORY,
                "Laderaum Inventar:");
        PL(JETTISON,
                "Abwerfen");
        PL(JETTISONED,
                "Abwerfen 1t: "); // preserve trailing space
        PL(COMBAT_RATING,
                "KAMPFWERTUNG:");
        PL(CRIMINAL_RECORD,
                "VORSTRAFENREGISTER:");
        PL(SHIP_INFORMATION_HEADER,
                "RAUMSCHIFF INFORMATION:  "); // preserve trailing space
        PL(HYPERDRIVE,
                "Hyperlaufwerk");
        PL(FREE,
                "Frei");
        PL(USED,
                "Benutzt");
        PL(TOTAL_WEIGHT,
                "Gesamtgewicht");
        PL(FRONT_WEAPON,
                "Vor Waffe");
        PL(REAR_WEAPON,
                "Hinten Waffe");
        PL(HYPERSPACE_RANGE,
                "Fahrbereich im Hyperraum");
        PL(NO_MOUNTING,
                "Kein Mountpunkt");
        PL(SHIP_INFORMATION,
                "Raumschiff Information");
        PL(REPUTATION,
                "Ruf");
        PL(MISSIONS,
                "Missionen");
        PL(SHIFT,
                "shift "); // preserve trailing space
        PL(CTRL,
                "strg "); // preserve trailing space
        PL(ALT,
                "alt "); // preserve trailing space
        PL(META,
                "meta "); // preserve trailing space
        PL(JOY,
                "Joy");
        PL(BUTTON,
                "-Taste "); // preserve leading and trailing space
        PL(HAT,
                "-Hut"); // preserve leading space
        PL(DIRECTION,
                "-Richtung "); // preserve leading and trailing space
        PL(X,
                "X");
        PL(Y,
                "Y");
        PL(Z,
                "Z");
        PL(AXIS,
                " Achse"); // preserve leading space
        PL(WEAPONS,
                "Waffen");
        PL(TARGET_OBJECT_IN_SIGHTS,
                "Ziel des Objekts in ein Fadenkreuz");
        PL(FIRE_LASER,
                "Laserkanone schießen");
        PL(SHIP_ORIENTATION,
                "Schiff Orientierung");
        PL(FAST_ROTATION_CONTROL,
                "schnelle Rotation Kontrolle");
        PL(PITCH_UP,
                "Kippen oben");
        PL(PITCH_DOWN,
                "Kippen unten");
        PL(YAW_LEFT,
                "Gieren links");
        PL(YAW_RIGHT,
                "Gieren rechts");
        PL(ROLL_LEFT,
                "Rollen links");
        PL(ROLL_RIGHT,
                "Rollen rechts");
        PL(MANUAL_CONTROL_MODE,
                "Handbetrieb");
        PL(THRUSTER_MAIN,
                "Thrust forward");
        PL(THRUSTER_RETRO,
                "Thrust backwards");
        PL(THRUSTER_VENTRAL,
                "Thrust up");
        PL(THRUSTER_DORSAL,
                "Thrust down");
        PL(THRUSTER_PORT,
                "Thrust left");
        PL(THRUSTER_STARBOARD,
                "Thrust right");
        PL(SPEED_CONTROL_MODE,
                "Speed control mode");
        PL(INCREASE_SET_SPEED,
                "Increase set speed");
        PL(DECREASE_SET_SPEED,
                "Decrease set speed");
        PL(JOYSTICK_INPUT,
                "Joystick input");
        PL(PITCH,
                "Kippen");
        PL(ROLL,
                "Rollen");
        PL(YAW,
                "Gieren");
        PL(MISSILE,
                "Rakete");
        PL(HARMLESS,
                "Harmlos");
        PL(MOSTLY_HARMLESS,
                "Meistens harmlos");
        PL(POOR,
                "Mies");
        PL(AVERAGE,
                "Durchschnittlich");
        PL(ABOVE_AVERAGE ,
                "Überdurchschnittlich");
        PL(COMPETENT,
                "Kompetent");
        PL(DANGEROUS,
                "Gefährlich");
        PL(DEADLY,
                "Tödlich");
        PL(ELITE,
                "ELITE");
        PL(SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS,
                "Simulation Evolution des Universums: %.1f Milliarde Jahre ;-)");
        PL(TOMBSTONE_EPITAPH,
                "Ruhe in Frieden, Kumpel");
        PL(MM_START_NEW_GAME_EARTH,
                "Neues Spiel, beginnt auf Erde");
        PL(MM_START_NEW_GAME_E_ERIDANI,
                "Neues Spiel, beginnt auf Epsilon Eridani");
        PL(MM_START_NEW_GAME_DEBUG,
                "Neues Spiel zu Debugging Punkt");
        PL(MM_LOAD_SAVED_GAME,
                "Laden von gespeicherten Spiel");
        PL(MM_QUIT,
                "Pioneer beenden");
        PL(SCREENSHOT_FILENAME_TEMPLATE,
                "Spielbild%08d.png");
        PL(PIONEERING_PILOTS_GUILD,
                "Pionierarbeit Piloten Gilde");
        PL(RIGHT_ON_COMMANDER,
                "Well done Kommandant! Ihre Bekämpfung Bewertung hat sich verbessert!");
        PL(ALERT_CANCELLED,
                "Alarm aufgehoben.");
        PL(SHIP_DETECTED_NEARBY,
                "Schiff entdeckt in der Nähe.");
        PL(DOWNGRADING_ALERT_STATUS,
                "Keine schießen für 60 Sekunden erkannt; herabstufung die Alarmbereitschaft.");
        PL(LASER_FIRE_DETECTED,
                "Waffenschießen entdeckt.");
        PL(SOMEWHERE_POLICE,
                "%s Polizei");
        PL(WE_HAVE_NO_BUSINESS_WITH_YOU,
                "Wir haben keine Geschäfte mit Ihnen jetzt.");
        PL(YOU_MUST_PAY_FINE_OF_N_CREDITS,
                "Wir tolerieren keine Verbrechen. Sie müssen eine Geldbuße von %s zahlen.");
        PL(PAY_THE_FINE_NOW,
                "Die Geldbuße zahlen jetzt.");
        PL(HANG_UP,
                "Auflegen.");
        PL(TRADING_ILLEGAL_GOODS,
                "Der Handel mit illegalen Waren");
        PL(UNLAWFUL_WEAPONS_DISCHARGE,
                "Rechtswidrige Waffen schießen");
        PL(PIRACY,
                "Piraterei");
        PL(MURDER,
                "Mord");
        PL(INDEPENDENT,
                "Unabhängig");
        PL(EARTH_FEDERATION,
                "Erde Föderation");
        PL(INDEPENDENT_CONFEDERATION,
                "Gemeinschaft Unabhängiger Sternensystemen");
        PL(EMPIRE,
                "Das Imperium");
        PL(NO_ESTABLISHED_ORDER,
                "Zügellosigkeit");
        PL(HARD_CAPITALIST,
                "Völlig Kapitalist - keine Regierung Sozialleistungen");
        PL(CAPITALIST,
                "Kapitalist");
        PL(MIXED_ECONOMY,
                "Gemischte Wirtschaftsform");
        PL(PLANNED_ECONOMY,
                "Planwirtschaft");
        PL(NO_CENTRAL_GOVERNANCE,
                "Ohne zentrale Verwaltung");
        PL(EARTH_FEDERATION_COLONIAL_RULE,
                "Erde Föderation Kolonialherrschaft");
        PL(EARTH_FEDERATION_DEMOCRACY,
                "Erde Föderation Demokratie");
        PL(IMPERIAL_RULE,
                "Imperialer Herrschaft");
        PL(LIBERAL_DEMOCRACY,
                "Liberale Demokratie");
        PL(SOCIAL_DEMOCRACY,
                "Sozialdemokratie");
        PL(CORPORATE_SYSTEM,
                "Herrschaft der Konzerne");
        PL(MILITARY_DICTATORSHIP,
                "Militärdiktatur");
        PL(COMMUNIST,
                "Kommunist");
        PL(PLUTOCRATIC_DICTATORSHIP,
                "Plutokratischen Diktatur");
        PL(VIOLENT_ANARCHY,
                "Störung - Insgesamt Governance durch bewaffnete Gruppierungen bestritten");
        PL(X_CANNOT_BE_TOLERATED_HERE,
                "%s kann hier nicht geduldet werden.");
        PL(SECTOR_X_Y,
                "Sektor: %d,%d");
        PL(DISTANCE_FUEL_TIME,
                "Abst. %.2f Lichtjahre (Kraftstoff benötigt: %dt | Zeitverlust: %.1fst.)");
        PL(CURRENT_SYSTEM,
                "Current system");
        PL(DISTANCE_FUEL_REQUIRED,
                "Abst. %.2f Lichtjahre (unzureichende Kraftstoff; benötigt: %dt)");
        PL(DISTANCE_OUT_OF_RANGE,
                "Abst. %.2f Lichtjahre (out of range)");
        PL(CANNOT_HYPERJUMP_WITHOUT_WORKING_DRIVE,
                "Sie können nicht durchführen Hypersprung weil Sie nicht ein funktionierende Hyperlaufwerk haben");
        PL(QUADRUPLE_SYSTEM,
                "VierfachSternsystem. "); // preserve trailing space
        PL(TRIPLE_SYSTEM,
                "DreifachSternsystem. "); // preserve trailing space
        PL(BINARY_SYSTEM,
                "DoppelSternsystem. "); // preserve trailing space
        PL(FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED,
                "Kraftstoffschaufel aktiv. Sie haben nun %d Tonnen Wasserstoff.");
        PL(CARGO_BAY_LIFE_SUPPORT_LOST,
                "Problem entdeckt: Leben Unterstützung den Frachtraum funktioniert nicht.");
        PL(NO_FREE_SPACE_FOR_ITEM,
                "Sie haben keinen freien Raum für diesen Artikel.");
        PL(SHIP_IS_FULLY_LADEN,
                "Ihr Schiff voll beladen ist.");
        PL(YOU_DO_NOT_HAVE_ANY_X,
                "Sie haben keine %s.");
        PL(REAR_VIEW,
                "Rückansicht");
        PL(EXTERNAL_VIEW,
                "Außenansicht");
        PL(NAVIGATION_STAR_MAPS,
                "Navigation und Sternkarten");
        PL(COMMS,
                "Kommunication");
        PL(GALAXY_SECTOR_VIEW,
                "Galaktische Sektor Blick");
        PL(SYSTEM_ORBIT_VIEW,
                "System Umkreisung Blick");
        PL(STAR_SYSTEM_INFORMATION,
                "Sternsystem Information");
        PL(GALACTIC_VIEW,
                "Milchstraße Blick");
        PL(NO_ALERT,
                "kein Alarm");
        PL(SHIP_NEARBY,
                "Raumschiff in der Nähe");
        PL(DOCKING_CLEARANCE_EXPIRED,
                "Andocken Erlaubnis ist abgelaufen. Wenn Sie andocken möchten, müssen Sie wiederholen Sie Ihre Anfrage.");
        PL(MESSAGE_FROM_X,
                "Nachricht von %s:");
        PL(SELECT_A_TARGET,
                "Wählen Sie ein Ziel");
        PL(FRONT,
                "Front");
        PL(REAR,
                "Heck");
        PL(POLICE_SHIP_REGISTRATION,
                "POLIZEI");
        PL(CLEARANCE_ALREADY_GRANTED_BAY_N,
                "Andocken Erlaubnis bereits gewährt. Fliegen zur Andockbuchten %d.");
        PL(CLEARANCE_GRANTED_BAY_N,
                "Andocken Erlaubnis gewährt. Fliegen zur Andockbuchten %d.");
        PL(CLEARANCE_DENIED_NO_BAYS,
                "Andocken Erlaubnis verweigert. iEs gibt keine freien Andockbuchten.");
        PL(ITEM_IS_OUT_OF_STOCK,
                "Dieser Artikel ist ausverkauft.");
        PL(BROWN_DWARF,
                "Brauner Zwerg substellares Objekt");
        PL(WHITE_DWARF,
                "Weißer Zwerg Sternrest");
        PL(STAR_M,
                "Klasse 'M' roter Stern");
        PL(STAR_K,
                "Klasse 'K' oranger Stern");
        PL(STAR_G,
                "Klasse 'G' gelbe Stern");
        PL(STAR_F,
                "Klasse 'F' weiße Stern");
        PL(STAR_A,
                "Klasse 'A' heiße weiße Stern");
        PL(STAR_B,
                "Hellen Klasse 'B' blaue Stern");
        PL(STAR_O,
                "Heiße, massive Klasse 'O' Stern");
        PL(STAR_M_GIANT,
                "Roter Riesenstern");
        PL(STAR_K_GIANT,
                "Oranger Riesenstern - Instabil");
        PL(STAR_G_GIANT,
                "Gelbe Riesenstern - Instabil");
        PL(STAR_AF_GIANT,
                "Weiße Riesenstern");
        PL(STAR_B_GIANT,
                "Blaue Riesenstern");
        PL(STAR_O_GIANT,
                "Heiße blaue Riesenstern");
        PL(STAR_M_SUPER_GIANT,
                "Roter Überriesen");
        PL(STAR_K_SUPER_GIANT,
                "Oranger Überriesen");
        PL(STAR_G_SUPER_GIANT,
                "Gelbe Überriesen");
        PL(STAR_AF_SUPER_GIANT,
                "Weiße Überriesen");
        PL(STAR_B_SUPER_GIANT,
                "Blaue Überriesen");
        PL(STAR_O_SUPER_GIANT,
                "Heiße blaue Überriesen");
        PL(STAR_M_HYPER_GIANT,
                "Roter Hyperriesen");
        PL(STAR_K_HYPER_GIANT,
                "Oranger Hyperriesen - Instabil");
        PL(STAR_G_HYPER_GIANT,
                "Gelbe Hyperriesen - Instabil");
        PL(STAR_AF_HYPER_GIANT,
                "Weiße Hyperriesen");
        PL(STAR_B_HYPER_GIANT,
                "Blaue Hyperriesen");
        PL(STAR_O_HYPER_GIANT,
                "Heiße blaue Hyperriesen");
        PL(STAR_M_WF,
                "Wolf-Rayet-Stern - Instabil");
        PL(STAR_B_WF,
                "Wolf-Rayet-Stern - bei Gefahr des Zusammenbruchs");
        PL(STAR_O_WF,
                "Wolf-Rayet-Stern - Zusammenbruch bevorstand");
        PL(STAR_S_BH,
                "Stellares Schwarzes Loch");
        PL(STAR_IM_BH,
                "Mittelschweres Schwarzes Loch");
        PL(STAR_SM_BH,
                "Supermassereiches Schwarzes Loch");
        PL(VERY_LARGE_GAS_GIANT,
                "Sehr großer Gasriese");
        PL(LARGE_GAS_GIANT,
                "Großer Gasriese");
        PL(MEDIUM_GAS_GIANT,
                "Mittelschwere Gasriese");
        PL(SMALL_GAS_GIANT,
                "Kleiner Gasriese");
        PL(ASTEROID,
                "Asteroid");
        PL(MASSIVE,
                "Massiver");
        PL(LARGE,
                "Großer");
        PL(TINY,
                "Weniger");
        PL(SMALL,
                "Kleiner");
        PL(COMMA_HIGHLY_VOLCANIC,
                ", hoch vulkanischer");
        PL(HIGHLY_VOLCANIC,
                "Hoch vulkanischer");
        PL(ICE_WORLD,
                " Eisplanet"); // preserve leading space
        PL(ROCKY_PLANET,
                " felsiger Planet"); // preserve leading space
        PL(OCEANICWORLD,
                " ozeanischer Planet"); // preserve leading space
        PL(PLANET_CONTAINING_LIQUID_WATER,
                " Planet mit flüssigem Wasser"); // preserve leading space
        PL(PLANET_WITH_SOME_ICE,
                " Planet mit ein wenig Eis"); // preserve leading space
        PL(ROCKY_PLANET_CONTAINING_COME_LIQUIDS,
                " felsiger Planet mit einigen Flüssigkeiten"); // preserve leading space
        PL(WITH_NO_SIGNIFICANT_ATMOSPHERE,
                " mit keinen signifikanten Atmosphäre"); // preserve leading space
        PL(TENUOUS,
                "sehr dünner");
        PL(THIN,
                "dünner");
        PL(THICK,
                "dicker");
        PL(VERY_DENSE,
                "sehr dichter");
        PL(WITH_A,
                " mit einer "); // preserve leading and trailing space
        PL(O2_ATMOSPHERE,
                " Sauerstoffatmosphäre"); // preserve leading space
        PL(CO2_ATMOSPHERE,
                " Kohlendioxid-Atmosphäre"); // preserve leading space
        PL(CO_ATMOSPHERE,
                " Kohlenmonoxid-Atmosphäre"); // preserve leading space
        PL(CH4_ATMOSPHERE,
                " Methan-Atmosphäre"); // preserve leading space
        PL(H_ATMOSPHERE,
                " Wasserstoff-Atmosphäre"); // preserve leading space
        PL(HE_ATMOSPHERE,
                " Helium-Atmosphäre"); // preserve leading space
        PL(AR_ATMOSPHERE,
                " Argon-Atmosphäre"); // preserve leading space
        PL(S_ATMOSPHERE,
                " Schwefel-Atmosphäre"); // preserve leading space
        PL(N_ATMOSPHERE,
                " Stickstoff-Atmosphäre"); // preserve leading space
        PL(AND_HIGHLY_COMPLEX_ECOSYSTEM,
                " und ein sehr komplexes Ökosystem."); // preserve leading space
        PL(AND_INDIGENOUS_PLANT_LIFE,
                " und einheimischen Pflanzen."); // preserve leading space
        PL(AND_INDIGENOUS_MICROBIAL_LIFE,
                " und einheimischen mikrobiellen Lebens."); // preserve leading space
        PL(ORBITAL_STARPORT,
                "Umkreisiger Raumschiff Hafen");
        PL(STARPORT,
                "Raumschiff Hafen");
        PL(UNKNOWN,
                "<unknown>");
        PL(UNEXPLORED_SYSTEM_NO_DATA,
                "Unerforschtes Sternsystem. Keine weiteren Daten verfügbar.");
        PL(SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS,
                "Kleine Prospektion. Keine registrierten Siedlungen.");
        PL(SMALL_INDUSTRIAL_OUTPOST,
                "Kleiner Industrie-Außenpost.");
        PL(SOME_ESTABLISHED_MINING,
                "Einige etablierte Bergbau.");
        PL(YOUNG_FARMING_COLONY,
                "Neu gegründeter landwirtschaftlicher Kolonie.");
        PL(INDUSTRIAL_COLONY,
                "Industrieller Kolonie.");
        PL(MINING_COLONY,
                "Bergbauer Kolonie.");
        PL(OUTDOOR_AGRICULTURAL_WORLD,
                "Im Freien landwirtschaftlicher Planet.");
        PL(HEAVY_INDUSTRY,
                "Schwerindustrie.");
        PL(EXTENSIVE_MINING,
                "Schwere Bergbau.");
        PL(THRIVING_OUTDOOR_WORLD,
                "Gut gehendlicher Planet mit Kolonien im Freien.");
        PL(INDUSTRIAL_HUB_SYSTEM,
                "Industrielles Knotenpunktsystem.");
        PL(VAST_STRIP_MINE,
                "Riesigen Tagebau-Kolonie.");
        PL(HIGH_POPULATION_OUTDOOR_WORLD,
                "Hohe Bevölkerung Planet mit Städte im Freien.");
        PL(SOMEWHERE_SPACEPORT,
                " Raumhafen"); // preserve leading space
        PL(SOMEWHERE_STARPORT,
                " Sternhafen"); // preserve leading space
        PL(MASS,
                "Masse");
        PL(N_WHATEVER_MASSES,
                "%.3f %smassen");
        PL(SOLAR,
                "Sonnen");
        PL(EARTH,
                "Erd");
        PL(SURFACE_TEMPERATURE,
                "Oberflächentemperatur");
        PL(N_CELSIUS,
                "%d C");
        PL(N_YEARS,
                "%.1f Jahre");
        PL(N_DAYS,
                "%.1f Tage");
        PL(ORBITAL_PERIOD,
                "Umlaufzeit");
        PL(PERIAPSIS_DISTANCE,
                "Periapsisdistanz");
        PL(APOAPSIS_DISTANCE,
                "Apoapsisdistanz");
        PL(ECCENTRICITY,
                "Bahnexzentrizität");
        PL(AXIAL_TILE,
                "Neigung der Rotationsachse");
        PL(N_DEGREES,
                "%.1f grad");
        PL(DAY_LENGTH,
                "Tageslänge");
        PL(N_EARTH_DAYS,
                "%.1f Erdentage");
        PL(STARPORTS,
                "Raumschiffhäfen");
        PL(MAJOR_IMPORTS,
                "Bedeutende Einfuhren:");
        PL(MINOR_IMPORTS,
                "Andere Einfuhren:");
        PL(MAJOR_EXPORTS,
                "Bedeutende Ausfuhren:");
        PL(MINOR_EXPORTS,
                "Andere Ausfuhren:");
        PL(ILLEGAL_GOODS,
                "Illegale Waren:");
        PL(UNEXPLORED_SYSTEM_STAR_INFO_ONLY,
                "Unerforschtes Sternsystem. Stern-information war per Fernbedienung Teleskop gesammelt worden, aber keine Information auf Planeten ist Verfügbar.");
        PL(PLANETARY_INFO,
                "Planeten-information");
        PL(ECONOMIC_INFO,
                "Wirtschaft-information");
        PL(DEMOGRAPHICS,
                "Demografie");
        PL(STABLE_SYSTEM_WITH_N_MAJOR_BODIES_STARPORTS,
                "Stabiles System mit %d großen %s and %d Raumschiffha%fen.\n\n%s"); // WHAT A MESS!
        PL(BODY,
                "Körpern");
        PL(BODIES,
                "Körper");
        PL(PLURAL_SUFFIX,
                "e"); // THIS IS A HUGE ASSUMPTION BASED ON ENGLISH.
        PL(SYSTEM_TYPE,
                "Systemtyp:");
        PL(GOVERNMENT_TYPE,
                "Regierung Typ:");
        PL(ECONOMY_TYPE,
                "Wirtschaft Typ:");
        PL(ALLEGIANCE,
                "Gefolgschaft:");
        PL(POPULATION,
                "Einwohnerzahl:");
        PL(OVER_N_BILLION,
                "Mehr als %d Milliarden");
        PL(OVER_N_MILLION,
                "Mehr als %d Millionen");
        PL(A_FEW_THOUSAND,
                "Nur einige Tausend");
        PL(NO_REGISTERED_INHABITANTS,
                "Keine registrierten Einwohner");
        PL(SECTOR_COORDINATES,
                "Koordinaten des Sektors:");
        PL(SYSTEM_NUMBER,
                "System-Nummer:");
        PL(NAME,
                "Name");
        PL(ROTATIONAL_PERIOD,
                " (Rotationsperiode)"); // preserve leading space
        PL(RADIUS,
                "Radius");
        PL(SEMI_MAJOR_AXIS,
                "Große Halbachse");
        PL(TIME_POINT,
                "Zeitpunkt: ");
        PL(UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW,
                "Unerforschtes Sternsystem. Sternsystem Blick unverfügbar.");
        PL(WHEELS_ARE_UP,
                "Fahrwerk angehoben");
        PL(WHEELS_ARE_DOWN,
                "Fahrwerk abgesenkt");
        PL(OBJECT_LABELS_ARE_ON,
                "Bezeichnungsschilder sichtbar");
        PL(OBJECT_LABELS_ARE_OFF,
                "Bezeichnungsschilder unsichtbar");
        PL(HYPERSPACE_JUMP,
                "Hyperraumsprung");
        PL(TAKEOFF,
                "Abflug");
        PL(MANUAL_CONTROL,
                "Manuelle Steuerung");
        PL(COMPUTER_SPEED_CONTROL,
                "Rechner Geschwindigkeitsregelung");
        PL(AUTOPILOT_ON,
                "Autopilot aktiviert");
        PL(SHIP_VELOCITY_BY_REFERENCE_OBJECT,
                "Schiff Geschwindigkeit von Referenzobjekt");
        PL(DISTANCE_FROM_SHIP_TO_NAV_TARGET,
                "Entfernung vom Schiff auf die Navigationsziel");
        PL(SHIP_ALTITUDE_ABOVE_TERRAIN,
                "Schiff Höhe über Gelände");
        PL(EXTERNAL_ATMOSPHERIC_PRESSURE,
                "Externe Atmosphärendruck");
        PL(HULL_TEMP,
                "Rumpftemperatur");
        PL(WEAPON_TEMP,
                "Waffentemperatur");
        PL(HULL_INTEGRITY,
                "Rumpfintegrität");
        PL(SHIELD_INTEGRITY,
                "Schildintegrität");
        PL(LAUNCH_PERMISSION_DENIED_BUSY,
                "Startberechtigung verweigert: Andockbucht beschäftigt.");
        PL(HYPERSPACE_JUMP_ABORTED,
                "Hyperraumsprung abgebrochen.");
        PL(CAMERA,
                "Fotoaparat");
        PL(LANDED,
                "Gelandet");
        PL(DOCKING,
                "Andocken");
        PL(DOCKED,
                "Angedockt");
        PL(HYPERSPACE,
                "Hyperraum");
        PL(SET_SPEED_KM_S,
                "Geschwindigkeit angesetzt: %.2f km/s");
        PL(SET_SPEED_M_S,
                "Geschwindigkeit angesetzt: %.0f m/s");
        PL(KM_S_RELATIVE_TO,
                "%.2f km/s bez-auf %s");
        PL(M_S_RELATIVE_TO,
                "%.0f m/s bez-auf %s");
        PL(N_DISTANCE_TO_TARGET,
                "%s nach Ziel");
        PL(ALT_IN_METRES,
                "Höhe: %.0fm");
        PL(PRESSURE_N_BAR,
                "Druck: %.2f bar");
        PL(NO_HYPERDRIVE,
                "Kein Hyperlaufwerk");
        PL(MASS_N_TONNES,
                "Masse: %dt");
        PL(SHIELD_STRENGTH_N,
                "Schild-Stärke: %.2f");
        PL(CARGO_N,
                "Ladung: %dt");
        PL(HYPERSPACE_ARRIVAL_CLOUD_REMNANT,
                "Hyperraum Anflug Wolke Rest");
        PL(HYPERSPACE_X_CLOUD,
                "Hyperraum %s Wolke");
        PL(SHIP_MASS_N_TONNES,
                "Schiff Masse: %dt");
        PL(DATE_DUE_N,
                "Fälligkeitsdatum: %s");
        PL(ARRIVAL,
                "Anflug");
        PL(DEPARTURE,
                "Abflug");
        PL(SOURCE,
                "Ursprung");
        PL(DESTINATION,
                "Reiseziel");
        PL(HYPERSPACING_IN_N_SECONDS,
                "Hyperraumsprung in %.0f Sekunden");
        PL(NAVIGATION_TARGETS_IN_THIS_SYSTEM,
                "Navigation Ziele innerhalb dieses Systems");
        PL(YOU_NO_MONEY,
                "Sie haben kein Geld.");
        PL(FINE_PAID_N_BUT_N_REMAINING,
                "Sie haben %s bezahlt, aber Sie müssen noch eine herausragende Geldstrafe von %s zu bezahlen.");
        PL(FINE_PAID_N,
                "Sie haben die Geldstrafe von %s bezahlt.");
        PL(SET_HYPERSPACE_DESTINATION_TO,
                "Set Hyperraum Ziel nach "); // preserve trailing space
        PL(NO_TARGET_SELECTED,
                "Schiffsrechner: Kein Ziel ausgewählt");
        PL(REQUEST_DOCKING_CLEARANCE,
                "Erlaubnis Ansuchen um zu andocken");
        PL(AUTOPILOT_DOCK_WITH_STATION,
                "Autopilot: mit Raumstation andocken");
        PL(PAY_FINE_REMOTELY,
                "Geldstrafe bezahlen durch Fernübertragung (%s)");
        PL(AUTOPILOT_FLY_TO_VICINITY_OF,
                "Autopilot: Fliegen nach der Nähe von "); // preserve trailing space
        PL(AUTOPILOT_ENTER_LOW_ORBIT_AROUND,
                "Autopilot: Flieg nach niedriger Umlaufbahn um "); // preserve trailing space
        PL(AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND,
                "Autopilot: Flieg nach mittlerer Umlaufbahn um "); // preserve trailing space
        PL(AUTOPILOT_ENTER_HIGH_ORBIT_AROUND,
                "Autopilot: Flieg nach hoher Umlaufbahn um "); // preserve trailing space
        PL(SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE,
                "Hyperraumwolke Analysator: Set Hyperraum Ziel um dieser Abfahrt zu folgen");
        PL(SET_HYPERSPACE_TARGET_TO,
                "Set Hyperraum Ziel um "); // preserve trailing space
    }
}
