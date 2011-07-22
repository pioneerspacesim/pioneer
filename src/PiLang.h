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
                "%s Shipyard");
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
                "%d t");
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
        PL(CONTACT_LOCAL_POLICE,
                "Contact local police");
        PL(COMMS_LINK,
                "Comms link");
 
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
                "An onboard flight computer.");
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

        // Heavenly bodies
    }
    namespace test {
        // An example additional language.  It inherits all strings from
        // the English namespace, and then overloads them.  This way,
        // untranslated strings fall back to English.
        using namespace en;
        PL(SUGGESTED_RESPONSES,
                "Tested responses:");
    }
}
