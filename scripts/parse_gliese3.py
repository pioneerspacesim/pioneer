#!/usr/bin/env python
#
# This script parses the gliese star catalogue (v3, 1991) and spits out custom system
# specifications for pioneer that you can stick in custom_starsystems.cpp
#
f = open("GLIESE3.DAT","r")

SECTOR_SIZE = 8.0 # ly
RANGE = 20.0 # ly
Z_RESOLVE = "mangle" # either "preserve", "cull", or "mangle"

# Name substitutions
namesubs = { "ALF CMa ADS 5423 LTT 2638": "Sirius",
"van Maanen 2": "Van Maanen's Star",
"ALF Cmi": "Procyon",
"ALF Cen": "Alpha Centauri",
"AC+58:25001 Stein 2051": "Stein 2051",
"EPS Eri": "Epsilon Eridani",
"Wolf 629   cpm with Gl 644  sep 72\" 315d": "Wolf 629",
"61 Cyg": "61 Cygni",
"TAU Cet": "Tau Ceti",
"CP -40:7021 LTT 6210   comp. B is optical": "Gliese 588",
"ALF Aql": "Altair",
"L 722-22 Hei 299  sep 0.14\" V(AB) = 11.50 d(m) = 0.5 :": "Hei 299",
"L 726-008 LDS 838  V(AB) = 11.89 d(m) = 0.14": "UV Ceti",
"ETA Cas": "Eta Cassiopeia",
"LTT 1907 OMI(2) Eri": "40 Eridani",
"DEL Pav": "Delta Pavonis",
"L 789-006   V(AB) = 12.30 d(m) = 1.0  ; a = 0.36\" P = 2.2 yr": "Luyten 789-006",
"LTT 12923 Wolf 359": "Wolf 359",
"AC-24:2833-183": "Ross 154",
"ADS 10417AB  V(AB) = 4.32 d(m) = 0.04": "ADS 10417",
"Ross 986 AC+38:23616": "Ross 986",
"V1581 Cyg  orbit in Harrington AJ 100, p 559 (1990)": "GJ 1245",
"Ross 614  V(AB) = 10.10 d(m) = 3.5 ?": "Ross 614",
"ADS 11046  V(AB) = 4.02 d(m) = 1.80": "70 Ophiuchi",
"ADS 10417C  cpm to Gl 663, sep 732\"": "Gliese 663",
"LFT 295 LTT 1702": "Luyten 372-58",
"SIG Dra": "Sigma Draconis",
"Proxima Cen": "Proxima",
"AD Leo": "AD Leonis",
"ADS 15972 Kr 60  V(AB) = 9.59 d(m) = 1.67": "Kruger 60",
"L 354-89 CD-49:13515 Sm 83": "Luyten 354-89",
"EV Lac   opt. comp. sep 5\"": "EV Lacertae",
"Wolf 424   V(AB) = 12.43 d(m) = 0.3": "Wolf 424",
"L 1159-016": "Luyten 1159-016",
"EPS Ind": "Epsilon Indi",
"GJ 1116": "EI Cancri",
"GJ 1111": "DX Cancri",
"GX And": "GX Andromedae",
"Gliese 411": "Lalande 21185",
"Gliese 273": "Luyten's Star",
"L 145-141": "Luyten 145-141",
"LTT 670 L 725-32": "YZ Ceti"
}

import re
from math import *
from random import *
PC_2_LY = 3.261633

starsystems = {}

def polar2cartesian(ascen, decl):
	return (sin(ascen)*cos(decl), sin(decl), cos(ascen)*cos(decl))
def cross(a,b):
	return [a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]]
def normalOf(a):
	l = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2])
	return [a[0]/l, a[1]/l, a[2]/l]
def matVecMul(m, v):
	out = [0,0,0]
	out[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2]
	out[1] = m[3]*v[0] + m[4]*v[1] + m[5]*v[2]
	out[2] = m[6]*v[0] + m[7]*v[1] + m[8]*v[2]
	return out
def vecNeg(v):
	return (-v[0], -v[1], -v[2])
# need to convert coords from earthto galactic
# rows first guvnah
galZ = normalOf(polar2cartesian(2*pi*12/24.0 + 2*pi*51/(24*60.0), 27.4*(pi/180.0)))
galX = vecNeg(normalOf(polar2cartesian(2*pi*17/24.0 + 2*pi*45/(24*60.0), -28.93*(pi/180.0))))
galY = normalOf(cross(galZ, galX))
#galX = normalOf(cross(galZ, galX))
galRot = (galX[0], galX[1], galX[2],
	galY[0], galY[1], galY[2],
	galZ[0], galZ[1], galZ[2])

while 1:
	s = f.readline()
	if not s: break
	s = s.replace("\r\n","")
	try:
		glieseName = s[0:8]
		name = s[188:]
		if not name:
			name = glieseName.strip().replace("Gl", "Gliese")
		if name in namesubs.keys():
			name = namesubs[name]
		parallax = float(s[108:114])/1000.0
		dist = PC_2_LY/parallax
		if (dist > RANGE): continue
		decl = int(s[21:24])
		decl_frac = float(s[25:29])/60.0
		if (decl<0): decl -= decl_frac
		else: decl += decl_frac
		decl *= pi/180.0

		ascen = 2*pi*float(s[12:14])/24.0
		ascen += 2*pi*float(s[15:17])/(24.0*60)
		ascen += 2*pi*float(s[18:20])/(24.0*60*60)

		pos = (dist*sin(ascen)*cos(decl), dist*sin(decl), dist*cos(ascen)*cos(decl))

		if Z_RESOLVE == "cull":
			if (fabs(pos[2]) > SECTOR_SIZE): continue
		elif Z_RESOLVE == "mangle":
			if (fabs(pos[2]) > SECTOR_SIZE):
				pos = cross(cross([0,0,1], pos),[0,0,1])
				pos = normalOf(pos)
				pos = [pos[0]*dist, pos[1]*dist, pos[2]*dist]
				# now all on x,y plane. give z some random shit and be done
				# (what a crap technique...
				pos[2] = (random()-0.5)*SECTOR_SIZE
		pos = matVecMul(galRot, pos)

		# make pioneer sector coords
		pcoords = [(pos[0]+4)/SECTOR_SIZE, (pos[1]+4)/SECTOR_SIZE, pos[2]/SECTOR_SIZE]
		spectral_class = s[54:66]
		wank = re.search("\s(D[ABOQZCX])(\d?)", spectral_class)
		if wank:
			spec = "StarSystem::TYPE_WHITE_DWARF"
			specnum = wank.group(2)
			spectral_class = wank.group(1)+specnum
		else:
			wank = re.search("s?d?([OBAFGKMobafgkm])(\d?)", spectral_class)
			if wank:
				spec = "StarSystem::TYPE_STAR_"+wank.group(1).upper()
				specnum = wank.group(2)
				spectral_class = wank.group(1).upper()+specnum
			else:
				import sys
				print >> sys.stderr, "Could not parse spectral class for ", name, ": ", spectral_class
				continue
		# sector
		sx = 0
		sy = 0
		while (pcoords[0] > 1): pcoords[0] -= 1 ; sx += 1
		while (pcoords[0] < 0): pcoords[0] += 1 ; sx -= 1
		while (pcoords[1] > 1): pcoords[1] -= 1 ; sy += 1
		while (pcoords[1] < 0): pcoords[1] += 1 ; sy -= 1
		if (starsystems.has_key(glieseName)):
			starsystems[glieseName]["components"].append(spectral_class)
			starsystems[glieseName]["spec"].append(spec)
		else:
			starsystems[glieseName] = { "name": name, "spec":[spec], "sx":sx, "sy":sy, "coords":pcoords, "components":[spectral_class] }

		#print "{ \"%s\", 0, StarSystem::TYPE_%s, %d, %d, vector3f(%.3f,%.3f,%.3f) }," % \
		#	(name.replace("\"","\\\""), spec, sx, sy, pcoords[0], pcoords[1], pcoords[2])
	except ValueError:
		pass

k = starsystems.keys()
for s in k:
	s = starsystems[s]
	comment = "// Components: " + ", ".join(s["components"])
	print "{ \"%s\", 0, {%s}, %d, %d, vector3f(%.3f,%.3f,%.3f) }, %s" % \
			(s["name"].replace("\"","\\\""), ", ".join(s["spec"]), s["sx"], s["sy"], s["coords"][0], s["coords"][1], s["coords"][2], comment)


