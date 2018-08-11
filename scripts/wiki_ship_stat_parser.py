#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json
#import csv

import math
import argparse
import sys
import os
from functools import reduce


def parse_arg():
    descrip = "Process json ship format to output in wiki format, (or csv)"
    epilog = """Example usage:

        /wiki_ship_stat_parser.py -e ../data/ships/
        Exports all json files in data/ships to ship.csv in same folder as script runs

        /wiki_ship_stat_parser.py -c ../data/ships/
        Exports all json files in data/ships to wiki table format to std out

        /wiki_ship_stat_parser.py -s ../data/ships/malabar.json
        Exports malabar.json data to to wiki infobox template format to std out"""

    parser = argparse.ArgumentParser(description=descrip, formatter_class=argparse.RawDescriptionHelpFormatter, epilog=epilog)

    parser.add_argument('read_path', help="Path to read data from", type=str)

    group = parser.add_mutually_exclusive_group()
    group.add_argument('-e','--export_from_path', action='store_true',
                        help='Export <path/*.json> to ships.csv')
    # feature not fully implemented yet:
    # group.add_argument('-i','--import_from_file', action='store_true',
    #                     help='Import from <file.csv>-file to <ship.json>')
    group.add_argument('-c','--chart_from_path',  action='store_true',
                        help='Print <path/*.json> to wiki chart format')
    group.add_argument('-s','--ship_from_file',   action='store_true',
                        help='Print <ship.json> to wiki table format')

    return parser.parse_args()

DELIM = "-"

d = { "one" : 1,
      "two" : 2,
      "abc" : {"foo" : -1, "bar" : -2},
      "three" : 3
    }

def flatten(b):
    "Flatten a json dictionary, to a single depth dictionary"
    val = {}
    for i in b.keys():
        if isinstance(b[i], dict):
            get = flatten(b[i])
            for j in get.keys():
                val[i + DELIM + j] = get[j]
        else:
            val[i] = b[i]
    return val

def inv_flatten(b, val={}):
    "Do the inverse of flatten"
    # TO-DO: recursive, now only can deal with depth one!

    for k in b.keys():
        atoms = k.split(DELIM)
        if len(atoms) == 1:
            val[k] = b[k]
        else:
            prefix = atoms[0]
            rest = reduce(lambda x,y: x+y, atoms[1:])
            if prefix not in val:
                val[prefix] = {}
            val[prefix][rest] = b[k]
    return val


# from data/libs/Equipset:
default_values = {
    "slots" + DELIM + "cargo" : 0,
    "slots" + DELIM + "engine" : 1,
    "slots" + DELIM + "laser_front" : 1,
    "slots" + DELIM + "laser_rear" : 0,
    "slots" + DELIM + "missile" : 0,
    "slots" + DELIM + "ecm" : 1,
    "slots" + DELIM + "radar" : 1,
    "slots" + DELIM + "target_scanner" : 1,
    "slots" + DELIM + "hypercloud" : 1,
    "slots" + DELIM + "hull_autorepair" : 1,
    "slots" + DELIM + "energy_booster" : 1,
    "slots" + DELIM + "atmo_shield" : 1,
    "slots" + DELIM + "cabin" : 50,
    "slots" + DELIM + "shield" : 9999,
    "slots" + DELIM + "scoop" : 2,
    "slots" + DELIM + "laser_cooler" : 1,
    "slots" + DELIM + "cargo_life_support" : 1,
    "slots" + DELIM + "autopilot" : 1,
    "slots" + DELIM + "trade_computer" : 1,
    "slots" + DELIM + "sensor " :  8,
    "slots" + DELIM + "thruster " :  1,

    "roles" + DELIM + "mercenary" : "NIL",
    "roles" + DELIM + "merchant" : "NIL",
    "roles" + DELIM + "pirate" : "NIL",

    "thrust_upgrades" + DELIM + "thruster_1" : "NIL",
    "thrust_upgrades" + DELIM + "thruster_2" : "NIL",
    "thrust_upgrades" + DELIM + "thruster_3" : "NIL",

    "tag" : "ship"
    # strange: "slots" + DELIM + "sensor" ???
}

def import_from_csv(csv_file):
    """Import a previously exported file of all ships,
    and return one valid json file for each row (ship)"""

    def myformat(s):
        "Convert string s to correct type"
        if s.isdigit():
            return int(s)
        elif s.lstrip('-').replace('.','',1).isdigit(): #s.replace('.','',1).isdigit():
            return float(s)
        elif s == "True":
            return True
        elif s == "False":
            return False
        else:
            return s

    data = []
    dic = {}

    with open(csv_file, 'r') as ships:
        header = ships.readline().split(',')
        print("header:", header)
        for line in ships:
            ship = line.split(',')
            filename = ship[0]
            dic[filename] = {}
            for i, field in enumerate(header[1:],1):
                if ship[i] != "NIL":
                    dic[filename][field] = myformat(ship[i])

            r = inv_flatten(dic[filename])
            del r['\n']         # don't know where this comes from, but remove it
            print(filename)
            print(r)
            with open("/tmp/" + filename + ".json", 'w') as f:
                json.dump(r, f, sort_keys=True, indent=4, separators=(',', ': '))


def export_to_csv(path):
    "Convert from json to csv"

    def add(data, isLast=False):
        s = str(data)
        if isLast:
            s += "\n"
        else:
            s += ","
        return s

    data = {}
    headers = set()

    # Read in all ship json-files, into simple "flat" dictionaries
    for filename in os.listdir(path):
        data[filename] = flatten(json.load(open(path + "/" + filename)))

        if "tag" in data[filename]:
            if data[filename]["tag"] == "static" or data[filename]["tag"] == "missile":
                del data[filename]

    # Iterate over all data, to see all possible headers
    for ship in data:
        for label in data[ship]:
            headers.add(label)

    f = open('ships.csv', 'w')

    # write header:
    f.write(add("filename"))
    for label in headers:
        f.write(add(label))
    f.write("\n")

    # write main data
    for ship in data:
        filename = ship.replace(".json", "")
        print("\n%s" % filename)
        row = "" + add(filename)
        for label in headers:
            try:
                if label in data[ship]:
                    # print(label, ":", data[ship][label])
                    row += add(data[ship][label])
                elif label in default_values:
                    # print(label, ":", default_values[label], "(default)")
                    row +=add(default_values[label])
                else:
                    row +=add("")
                    print("Unknown label:", filename, label)
            except:
                print(label, ":", "NIL")
                row += add("NIL")
        row += "\n"
        f.write(row)
    f.close()

def load_sketchfab(filename):
    with open(filename, 'r') as data_file:
        return json.load(data_file)

def get_ship_type(s):
    return "%s [[File:ship_class_%s.png]]" % (s.replace('_', ' ').capitalize(), s)

# hard coded mapping:
manufacturers = {
    "albr" : "Albr Corp",
    "auronox" : "Auronox Corporation",
    "haber" : "Haber Corporation",
    "kaluri" : "OKB Kaluri",
    "mandarava_csepel" : "Mandarava-Csepel",
    "opli" : "OPLI-Barnard Inc.",
}


def usage_error(arg):
    print("usage:")
    print("Needs one argument: path to full file")
    print(arg[0], "../data/ships/<ship_script_name>.json")
    sys.exit(1)

def parse_file(shipname):
    "parse a single ship file"

    with open(shipname, 'r') as data_file:
        shipjson = json.load(data_file)

    return shipjson

def thrust(thrust, hull_mass, fuel_tank_mass, capacity = 0):
    "Default to computing thrust of empty ship"

    this_thrust = thrust / (9.81*1000*(hull_mass + fuel_tank_mass + capacity))
    return this_thrust


def myround(number, significant=1):
    factor = 10.0**significant
    return factor * int(number / factor + 0.5)

def print_ship_as_wiki_infobox(ship_name):
    "Write a given <ship>.json file to Infobox_Ship tempate for wiki"

    with open(ship_name, 'r') as ship_file:
        shipTable = json.load(ship_file)

    try:
        if shipTable['slots']['engine'] > 0:
            max_engine = "yes"
        else:
            max_engine = "no"
    except:
        # default to 1, so if not defined, it exists:
        max_engine = "yes"

    # calculate effective exhaust velocity
    deltaV_empty = shipTable['effective_exhaust_velocity'] * \
                   math.log((shipTable['hull_mass'] + shipTable['fuel_tank_mass'])
                            / shipTable['hull_mass'])

    deltaV_full = shipTable['effective_exhaust_velocity'] * \
                  math.log((shipTable['hull_mass'] + shipTable['fuel_tank_mass'] + shipTable['capacity']) /
                           (shipTable['hull_mass'] + shipTable['capacity']))

#    deltaV_empty = myround(deltaV_empty, -3) / 1000
#    deltaV_full = myround(deltaV_full, -3) / 1000
    deltaV_empty = round(deltaV_empty/1000)
    deltaV_full = round(deltaV_full/1000)

    print('{{Infobox_Ship')
    print("|name = %s" % shipTable['name'])
    print("|type = %s" % get_ship_type(shipTable['ship_class']))
    sketchfab = load_sketchfab("sketchfab.json")
    model = shipTable['model']
    if model in sketchfab:
        print("|sketchfab = <sketchfab>%s</sketchfab>" % sketchfab[model])
    else:
        print("|sketchfab = ''")
    print("|manufacturer = %s" % manufacturers[shipTable['manufacturer']])
    print("|effective_exhaust_velocity_empty = %skm/s\n|effective_exhaust_velocity_full = %skm/s" % (nice_int(deltaV_empty), nice_int(deltaV_full)))
    for d in ['forward', 'reverse', 'up', 'down', 'left', 'right', 'angular']:
        thrust_n = shipTable[d + '_thrust']
        [thrust_rounded,thrust_suffix] = get_si_suffix(thrust_n)
        empty    = thrust(thrust_n, shipTable['hull_mass'], shipTable['fuel_tank_mass'])
        full     = thrust(thrust_n, shipTable['hull_mass'], shipTable['fuel_tank_mass'], shipTable['capacity'])
        print("|%s_thrust_empty = %.1fG (%.2f%sN)\n|%s_thrust_full = %.1fG" % (d, empty, thrust_rounded, thrust_suffix, d, full))

    print("|max_cargo = %s" % nice_int(shipTable['slots']['cargo']))
    print("|max_laser = %s" % shipTable['slots']['laser_front'])
    print("|max_missile = %s" % shipTable['slots']['missile'])
    print("|max_scoop_mounts = %s" % shipTable['slots']['scoop'])
    print("|hyperdrive_class = %s" % shipTable['hyperdrive_class'])
    print("|min_crew = %s" % shipTable['min_crew'])
    print("|max_crew = %s" % shipTable['max_crew'])
    print("|capacity = %s" % nice_int(shipTable['capacity']))
    print("|hull_mass = %s" % nice_int(shipTable['hull_mass']))
    print("|fuel_tank_mass = %s" % nice_int(shipTable['fuel_tank_mass']))
    # https://stackoverflow.com/questions/1823058/how-to-print-number-with-commas-as-thousands-separators#10742904
    print("|price = %s" % nice_int(shipTable['price']))
    print("|max_engine = " + max_engine)
    print("}}")

def nice_int(n):
    return "{:,}".format(int(n))


def make_chart(ship_folder):
    "Make wiki table from ships/*.json path"

    ships = {}
    # Read in all ship json-files, into simple "flat" dictionaries
    for filename in os.listdir(ship_folder):
        flying_thing = flatten(json.load(open(ship_folder + "/" + filename)))
        if 'ship_class' in flying_thing and flying_thing['price'] > 0:
            ships[filename] = flying_thing
        else:
            print("skipping:", flying_thing['name'])

    print("\nCOPY AND PASTE BELOW:\n")

    print('{| class="wikitable sortable"') # table start

    # Print wiki header (prefixed by "!")
    print("! Name")
    print("!")
    print("! Ship Class")
    print("! Laser Mounts")
    print("! Missile Capacity")
    print("! Cargo Capacity (t)")
    print("! Forward Thrust (G)")
    print("! Angular Thrust (G)")
    print("! Price")

    # for each print stats
    for filename in sorted(ships):
        ship_table = ships[filename]
        forward_thrust = ship_table['forward_thrust']
        forward_thrust_empty = round(thrust(forward_thrust,
                                            ship_table['hull_mass'],
                                            ship_table['fuel_tank_mass']), 1)
        angular_thrust = ship_table['angular_thrust']
        angular_thrust_empty = round(thrust(angular_thrust,
                                            ship_table['hull_mass'],
                                            ship_table['fuel_tank_mass']), 1)
        [fwd_thrust_rounded,fwd_thrust_suffix] = get_si_suffix(forward_thrust)
        [ang_thrust_rounded,ang_thrust_suffix] = get_si_suffix(angular_thrust)
        print("|-")             # new table row
        name = ships[filename]['name']
        print("|[[" + name.replace(" ", "_") + "|" + name + "]]")
        shipclass = ships[filename]['ship_class']
        print("|" + "[[File:Ship_class_" + shipclass + ".png]]")
        print("|" + shipclass.replace("_"," ").capitalize())
        print("| %s" % ships[filename]['slots-laser_front'])
        print("| %s" % ships[filename]['slots-missile'])
        print("| %s" % nice_int(ships[filename]['capacity']))
        print("| %s (%.0f%sN)" % (forward_thrust_empty, fwd_thrust_rounded, fwd_thrust_suffix))
        print("| %s (%.0f%sN)" % (angular_thrust_empty, ang_thrust_rounded, ang_thrust_suffix))
        print("| %s" % nice_int(ships[filename]['price']))

    print('|}')                 # table end


# https://stackoverflow.com/questions/10969759/python-library-to-convert-between-si-unit-prefixes#10970888
def get_si_suffix(number):
    "Return the number and the proper si suffix. 123000 -> [123,'k']"
    si = {
        -18 : {'multiplier' : 10 ** 18, 'prefix' : 'a'},
        -17 : {'multiplier' : 10 ** 18, 'prefix' : 'a'},
        -16 : {'multiplier' : 10 ** 18, 'prefix' : 'a'},
        -15 : {'multiplier' : 10 ** 15, 'prefix' : 'f'},
        -14 : {'multiplier' : 10 ** 15, 'prefix' : 'f'},
        -13 : {'multiplier' : 10 ** 15, 'prefix' : 'f'},
        -12 : {'multiplier' : 10 ** 12, 'prefix' : 'p'},
        -11 : {'multiplier' : 10 ** 12, 'prefix' : 'p'},
        -10 : {'multiplier' : 10 ** 12, 'prefix' : 'p'},
        -9 : {'multiplier' : 10 ** 9, 'prefix' : 'n'},
        -8 : {'multiplier' : 10 ** 9, 'prefix' : 'n'},
        -7 : {'multiplier' : 10 ** 9, 'prefix' : 'n'},
        -6 : {'multiplier' : 10 ** 6, 'prefix' : 'u'},
        -5 : {'multiplier' : 10 ** 6, 'prefix' : 'u'},
        -4 : {'multiplier' : 10 ** 6, 'prefix' : 'u'},
        -3 : {'multiplier' : 10 ** 3, 'prefix' : 'm'},
        -2 : {'multiplier' : 10 ** 2, 'prefix' : 'c'},
        -1 : {'multiplier' : 10 ** 1, 'prefix' : 'd'},
        0 : {'multiplier' : 1, 'prefix' : ''},
        1 : {'multiplier' : 10 ** 1, 'prefix' : 'd'},
        2 : {'multiplier' : 10 ** 3, 'prefix' : 'k'},
        3 : {'multiplier' : 10 ** 3, 'prefix' : 'k'},
        4 : {'multiplier' : 10 ** 3, 'prefix' : 'k'},
        5 : {'multiplier' : 10 ** 3, 'prefix' : 'k'},
        6 : {'multiplier' : 10 ** 6, 'prefix' : 'M'},
        7 : {'multiplier' : 10 ** 6, 'prefix' : 'M'},
        8 : {'multiplier' : 10 ** 6, 'prefix' : 'M'},
        9 : {'multiplier' : 10 ** 9, 'prefix' : 'G'},
        10 : {'multiplier' : 10 ** 9, 'prefix' : 'G'},
        11 : {'multiplier' : 10 ** 9, 'prefix' : 'G'},
        12 : {'multiplier' : 10 ** 12, 'prefix' : 'T'},
        13 : {'multiplier' : 10 ** 12, 'prefix' : 'T'},
        14 : {'multiplier' : 10 ** 12, 'prefix' : 'T'},
        15 : {'multiplier' : 10 ** 15, 'prefix' : 'P'},
        16 : {'multiplier' : 10 ** 15, 'prefix' : 'P'},
        17 : {'multiplier' : 10 ** 15, 'prefix' : 'P'},
        18 : {'multiplier' : 10 ** 18, 'prefix' : 'E'},
        }
        # Checking if its negative or positive
    if number < 0:
        negative = True;
    else:
        negative = False;

    # if its negative converting to positive (math.log()....)
    if negative:
        number = number - (number*2);

    # Taking the exponent
    exponent = int(math.log10(number));

    # Checking if it was negative converting it back to negative
    if negative:
        number = number - (number*2);

    # If the exponent is smaler than 0 dividing the exponent with -1
    if exponent < 0:
        exponent = exponent-1;
        return [number * si[exponent]['multiplier'], si[exponent]['prefix']];
    # If the exponent bigger than 0 just return it
    elif exponent > 0:
        return [number / si[exponent]['multiplier'], si[exponent]['prefix']];
    # If the exponent is 0 than return only the value
    elif exponent == 0:
        return [number, ''];

def main(args):

    if args.ship_from_file:
        print_ship_as_wiki_infobox(args.read_path)
    elif args.export_from_path:
        export_to_csv(args.read_path)
    elif args.chart_from_path:
        make_chart(args.read_path)
    elif args.import_from_file:
        import_from_csv(args.read_path)
    return 0


if __name__ == "__main__":
    args = parse_arg()

    main(args)
