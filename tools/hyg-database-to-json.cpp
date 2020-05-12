// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "basic_cbor.h"
#include "core/LZ4Format.h"

#include "argh/argh.h"
#include "csv-parser/csv.hpp"
#include "json/json.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <streambuf>

using Json = nlohmann::json;
using csv::CSVReader;
using csv::CSVRow;
using nonstd::string_view;
using std::to_string;

struct StarData {
	int32_t starID;
	float x, y, z;
	std::string spectralType;
	float absoluteMagnitude;
	std::string starName;
	std::string constellation;
	float colorIndex;

	Json to_json();
	void to_cbor(std::vector<uint8_t> &out);
};

// Convert superscript indexes into unicode characters
static const std::vector<std::string> superscript = {
	"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"
};

// A list of constellation abbreviations
static const std::map<std::string, std::string> greek = {
	{ "Alp", "Alpha" },
	{ "Bet", "Beta" },
	{ "Gam", "Gamma" },
	{ "Del", "Delta" },
	{ "Eps", "Epsilon" },
	{ "Zet", "Zeta" },
	{ "Eta", "Eta" },
	{ "The", "Theta" },
	{ "Iot", "Iota" },
	{ "Kap", "Kappa" },
	{ "Lam", "Lambda" },
	{ "Mu", "Mu" },
	{ "Nu", "Nu" },
	{ "Xi", "Xi" },
	{ "Omi", "Omicron" },
	{ "Pi", "Pi" },
	{ "Rho", "Rho" },
	{ "Sig", "Sigma" },
	{ "Tau", "Tau" },
	{ "Ups", "Upsilon" },
	{ "Phi", "Phi" },
	{ "Chi", "Chi" },
	{ "Psi", "Psi" },
	{ "Ome", "Omega" },
};

// A list of constellations and their nominative and genitive names
// Thanks to ecraven!
static const std::map<std::string, std::pair<std::string, std::string>> constellations = {
	{ "And", { "Andromeda", "Andromedae" } },
	{ "Ant", { "Antlia", "Antliae" } },
	{ "Aps", { "Apus", "Apodis" } },
	{ "Aqr", { "Aquarius", "Aquarii" } },
	{ "Aql", { "Aquila", "Aquilae" } },
	{ "Ara", { "Ara", "Arae" } },
	{ "Ari", { "Aries", "Arietis" } },
	{ "Aur", { "Auriga", "Aurigae" } },
	{ "Boo", { "Boötes", "Boötis" } },
	{ "Cae", { "Caelum", "Caeli" } },
	{ "Cam", { "Camelopardalis", "Camelopardalis" } },
	{ "Cnc", { "Cancer", "Cancri" } },
	{ "CVn", { "Canes Venatici", "Canum Venaticorum" } },
	{ "CMa", { "Canis Major", "Canis Majoris" } },
	{ "CMi", { "Canis Minor", "Canis Minoris" } },
	{ "Cap", { "Capricornus", "Capricorni" } },
	{ "Car", { "Carina", "Carinae" } },
	{ "Cas", { "Cassiopeia", "Cassiopeiae" } },
	{ "Cen", { "Centaurus", "Centauri" } },
	{ "Cep", { "Cepheus", "Cephei" } },
	{ "Cet", { "Cetus", "Ceti" } },
	{ "Cha", { "Chamaeleon", "Chamaeleontis" } },
	{ "Cir", { "Circinus", "Circini" } },
	{ "Col", { "Columba", "Columbae" } },
	{ "Com", { "Coma Berenices", "Comae Berenices" } },
	{ "CrA", { "Corona Australis", "Coronae Australis" } },
	{ "CrB", { "Corona Borealis", "Coronae Borealis" } },
	{ "Crv", { "Corvus", "Corvi" } },
	{ "Crt", { "Crater", "Crateris" } },
	{ "Cru", { "Crux", "Crucis" } },
	{ "Cyg", { "Cygnus", "Cygni" } },
	{ "Del", { "Delphinus", "Delphini" } },
	{ "Dor", { "Dorado", "Doradus" } },
	{ "Dra", { "Draco", "Draconis" } },
	{ "Equ", { "Equuleus", "Equulei" } },
	{ "Eri", { "Eridanus", "Eridani" } },
	{ "For", { "Fornax", "Fornacis" } },
	{ "Gem", { "Gemini", "Geminorum" } },
	{ "Gru", { "Grus", "Gruis" } },
	{ "Her", { "Hercules", "Herculis" } },
	{ "Hor", { "Horologium", "Horologii" } },
	{ "Hya", { "Hydra", "Hydrae" } },
	{ "Hyi", { "Hydrus", "Hydri" } },
	{ "Ind", { "Indus", "Indi" } },
	{ "Lac", { "Lacerta", "Lacertae" } },
	{ "Leo", { "Leo", "Leonis" } },
	{ "LMi", { "Leo Minor", "Leonis Minoris" } },
	{ "Lep", { "Lepus", "Leporis" } },
	{ "Lib", { "Libra", "Librae" } },
	{ "Lup", { "Lupus", "Lupi" } },
	{ "Lyn", { "Lynx", "Lyncis" } },
	{ "Lyr", { "Lyra", "Lyrae" } },
	{ "Men", { "Mensa", "Mensae" } },
	{ "Mic", { "Microscopium", "Microscopii" } },
	{ "Mon", { "Monoceros", "Monocerotis" } },
	{ "Mus", { "Musca", "Muscae" } },
	{ "Nor", { "Norma", "Normae" } },
	{ "Oct", { "Octans", "Octantis" } },
	{ "Oph", { "Ophiuchus", "Ophiuchi" } },
	{ "Ori", { "Orion", "Orionis" } },
	{ "Pav", { "Pavo", "Pavonis" } },
	{ "Peg", { "Pegasus", "Pegasi" } },
	{ "Per", { "Perseus", "Persei" } },
	{ "Phe", { "Phoenix", "Phoenicis" } },
	{ "Pic", { "Pictor", "Pictoris" } },
	{ "Psc", { "Pisces", "Piscium" } },
	{ "PsA", { "Piscis Austrinus", "Piscis Austrini" } },
	{ "Pup", { "Puppis", "Puppis" } },
	{ "Pyx", { "Pyxis", "Pyxidis" } },
	{ "Ret", { "Reticulum", "Reticuli" } },
	{ "Sge", { "Sagitta", "Sagittae" } },
	{ "Sgr", { "Sagittarius", "Sagittarii" } },
	{ "Sco", { "Scorpius", "Scorpii" } },
	{ "Scl", { "Sculptor", "Sculptoris" } },
	{ "Sct", { "Scutum", "Scuti" } },
	{ "Ser", { "Serpens", "Serpentis" } },
	{ "Sex", { "Sextans", "Sextantis" } },
	{ "Tau", { "Taurus", "Tauri" } },
	{ "Tel", { "Telescopium", "Telescopii" } },
	{ "Tri", { "Triangulum", "Trianguli" } },
	{ "TrA", { "Triangulum Australe", "Trianguli Australis" } },
	{ "Tuc", { "Tucana", "Tucanae" } },
	{ "UMa", { "Ursa Major", "Ursae Majoris" } },
	{ "UMi", { "Ursa Minor", "Ursae Minoris" } },
	{ "Vel", { "Vela", "Velorum" } },
	{ "Vir", { "Virgo", "Virginis" } },
	{ "Vol", { "Volans", "Volantis" } },
	{ "Vul", { "Vulpecula", "Vulpeculae" } }
};

std::string convert_bayer_flamsteed(std::string name)
{
	// convert bayer-flamsteed
	static std::regex matcher(
		R"((\d+)?([a-zA-Z]+)? *(\d)?(\w{3}))",
		std::regex::optimize);

	std::smatch results;
	if (!std::regex_search(name, results, matcher)) {
		std::cerr << "Could not parse B-F name " << name << std::endl;
		return "";
	}

	std::string out;

	// Flamsteed number
	if (results[1].length())
		out = results[1].str() + " ";

	// Bayer greek letter
	if (results[2].length() && results[2].str()[0] != ' ') {
		if (!greek.count(results[2].str()))
			std::cerr << "Unknown greek abbreviation " << results[2].str() << std::endl;
		out += greek.at(results[2].str());
	}

	if (results[3].length() && results[3].str()[0] != ' ') {
		char c = results[3].str()[0];

		// convert ASCII to 0-9
		int idx = c - 48;
		if (idx < 0 || idx > 9) {
			std::cerr << "Encountered invalid character '" << c << "' when parsing B-F desc: " << name << std::endl;
		} else {
			out += superscript[idx];
		}
	}

	if (!constellations.count(results[4].str()))
		std::cerr << "Unknown constellation abbreviation " << results[4].str() << " when building B-F name " << out << std::endl;
	return out + " " + constellations.at(results[4].str()).second;
}

/*
	# Fields in the HYG database:
	(note: field indicies were taken and updated from the GH page and may not be 100% correct)

	0  id: The database primary key.
	1  hip: The star's ID in the Hipparcos catalog, if known.
	2  hd: The star's ID in the Henry Draper catalog, if known.
	3  hr: The star's ID in the Harvard Revised catalog, which is the same as its number in the Yale Bright Star Catalog.
	4  gl: The star's ID in the third edition of the Gliese Catalog of Nearby Stars.
	5  bf: The Bayer / Flamsteed designation, primarily from the Fifth Edition of the Yale Bright Star Catalog. This is a combination of the two designations. The Flamsteed number, if present, is given first; then a three-letter abbreviation for the Bayer Greek letter; the Bayer superscript number, if present; and finally, the three-letter constellation abbreviation. Thus Alpha Andromedae has the field value "21Alp And", and Kappa1 Sculptoris (no Flamsteed number) has "Kap1Scl".
	6  proper: A common name for the star, such as "Barnard's Star" or "Sirius". I have taken these names primarily from the Hipparcos project's web site, which lists representative names for the 150 brightest stars and many of the 150 closest stars. I have added a few names to this list. Most of the additions are designations from catalogs mostly now forgotten (e.g., Lalande, Groombridge, and Gould ["G."]) except for certain nearby stars which are still best known by these designations.
	7  ra, dec: The star's right ascension and declination, for epoch and equinox 2000.0.
	9  dist: The star's distance in parsecs, the most common unit in astrometry. To convert parsecs to light years, multiply by 3.262. A value >= 100000 indicates missing or dubious (e.g., negative) parallax data in Hipparcos.
	10 pmra, pmdec: The star's proper motion in right ascension and declination, in milliarcseconds per year.
	12 rv: The star's radial velocity in km/sec, where known.
	13 mag: The star's apparent visual magnitude.
	14 absmag: The star's absolute visual magnitude (its apparent magnitude from a distance of 10 parsecs).
	15 spect: The star's spectral type, if known.
	16 ci: The star's color index (blue magnitude - visual magnitude), where known.
	17 x,y,z: The Cartesian coordinates of the star, in a system based on the equatorial coordinates as seen from Earth. +X is in the direction of the vernal equinox (at epoch 2000), +Z towards the north celestial pole, and +Y in the direction of R.A. 6 hours, declination 0 degrees.
	20 vx,vy,vz: The Cartesian velocity components of the star, in the same coordinate system described immediately above. They are determined from the proper motion and the radial velocity (when known). The velocity unit is parsecs per year; these are small values (around 1 millionth of a parsec per year), but they enormously simplify calculations using parsecs as base units for celestial mapping.
	23 rarad, decrad, pmrarad, prdecrad: The positions in radians, and proper motions in radians per year.
	27 bayer: The Bayer designation as a distinct value
	28 flam: The Flamsteed number as a distinct value
	29 con: The standard constellation abbreviation
	30 comp, comp_primary, base: Identifies a star in a multiple star system. comp = ID of companion star, comp_primary = ID of primary star for this component, and base = catalog ID or name for this multi-star system. Currently only used for Gliese stars.
	33 lum: Star's luminosity as a multiple of Solar luminosity.
	34 var: Star's standard variable star designation, when known.
	35 var_min, var_max: Star's approximate magnitude range, for variables. This value is based on the Hp magnitudes for the range in the original Hipparcos catalog, adjusted to the V magnitude scale to match the "mag" field.
*/

std::string get_star_name(const CSVRow &row)
{
	// Proper name
	if (row[6].is_str())
		return row[6].get();

	// Compressed Bayer-Flamsteed designation
	// e.g. 85 Peg -> 85 Pegasus
	// e.g. Kap1Scl -> Kappa¹ Sculptoris
	if (row[5].is_str()) {
		try {
			std::string name = convert_bayer_flamsteed(row[5].get());
			if (!name.empty())
				return name;
		} catch (std::exception &e) {
			std::cerr << "Encountered error " << e.what() << " parsing Bayer-Flamsteed description " << row[5].get() << std::endl;
		}
	}

	// Gliese catalog definition
	// e.g. Gl 914A
	if (row[4].is_str()) {
		return row[4].get();
	}

	// Henry Draper catalog number
	// e.g. 21406 -> HD 21406
	if (row[2].is_int()) {
		return "HD " + to_string(row[2].get<uint32_t>());
	}

	// Yale / Harvard catalog number
	// e.g. 21406 -> HR 21406
	if (row[3].is_int()) {
		return "HR " + to_string(row[3].get<uint32_t>());
	}

	// Hipparcos catalog number
	// e.g. 11 -> H 11
	if (row[1].is_int()) {
		return "HIP " + to_string(row[1].get<uint32_t>());
	}

	return "INV " + to_string(row[0].get<uint32_t>());
}

std::vector<StarData> parse_csv(const std::string &filename, CSVReader &reader)
{
	const auto &column_names = reader.get_col_names();
	if (column_names.empty()) {
		std::cerr << "Input file " << filename << " is not in CSV format!" << std::endl;
		return {};
	}

	if (column_names[0] != "id" && (column_names.size() < 31 || column_names[6] != "proper")) {
		std::cerr << "Invalid database format in file " + filename << std::endl;
		std::cerr << "Expected HYG v3 database format." << std::endl;
		std::cerr << "Got format: ";
		for (const auto &name : column_names) {
			std::cerr << name << ", ";
		}
		std::cerr << std::endl;

		return {};
	}

	std::vector<StarData> returnData;
	std::for_each(reader.begin(), reader.end(), [&](const CSVRow &row) {
		StarData this_star{};

		this_star.starID = row[0].get<uint32_t>();
		this_star.starName = get_star_name(row);

		try {
			if (row[15].is_str())
				this_star.spectralType = row[15].get();
			else // if we don't have a spectral type, assume it's a "standard" main-sequence M-class
				this_star.spectralType = "M3V";
			this_star.absoluteMagnitude = row[14].get<float>();

			// if we don't have a color index, default to approx. a G0V class star
			this_star.colorIndex = row[16].is_num() ? row[16].get<float>() : 0.58f;
		} catch (std::runtime_error &e) {
			std::cerr << "Error parsing star data for star " << this_star.starID << std::endl;
			std::cerr << e.what() << std::endl;
			return;
		}

		string_view con = row[29].get<string_view>();
		if (con.size() >= 3) // should only ever be 3 or 0, but manually enforce the length anyways
			this_star.constellation = con.substr(0, 3).to_string();
		else
			this_star.constellation = "";

		try {
			this_star.x = row[17].get<float>();
			this_star.y = row[18].get<float>();
			this_star.z = row[19].get<float>();
		} catch (std::runtime_error &e) {
			std::cerr << "Error parsing star location for star " << this_star.starID << std::endl;
			std::cerr << e.what() << std::endl;
			return;
		}

		returnData.push_back(std::move(this_star));
	});

	return returnData;
}

static const float PARSEC_TO_LY = 3.261564;

int sector_coord(float x)
{
	return int(std::floor(x * PARSEC_TO_LY)) / 8;
}

float system_loc(float x)
{
	return std::fmod(x * PARSEC_TO_LY, 8.0);
}

// Object format for JSON / CBOR star data:
/*
	{
		"name": "HD 12353",
		"absmag": 1235.04,
		"spectral": "GVIII",
		"color": 1.5031,
		"constl": "Pegasus",
		"sector": [ 4, 5, 6 ],
		"coords": [ 5.42134, 0.2145, 7.999 ]
	}
*/

// while it might be theoretically easier to simply use Json::to_cbor,
// this method is 1.6x faster and saves a bit of memory by using floats
// instead of only doubles.
void StarData::to_cbor(std::vector<uint8_t> &out)
{
	// remember to update the number of fields when adding more to the StarData type.
	cbor::push_tag(out, CBORTag::Object, 7);
	cbor::push_string(out, "name");
	cbor::push_string(out, starName);

	cbor::push_string(out, "absmag");
	cbor::push_float(out, absoluteMagnitude);

	cbor::push_string(out, "spectral");
	cbor::push_string(out, spectralType);

	cbor::push_string(out, "color");
	cbor::push_float(out, colorIndex);

	cbor::push_string(out, "constl");
	cbor::push_string(out, constellation);

	cbor::push_string(out, "sector");
	cbor::push_tag(out, CBORTag::Array, 3);
	{
		cbor::push_int(out, sector_coord(x));
		cbor::push_int(out, sector_coord(y));
		cbor::push_int(out, sector_coord(z));
	}

	cbor::push_string(out, "coords");
	cbor::push_tag(out, CBORTag::Array, 3);
	{
		cbor::push_float(out, std::fmod(x, 8.0f));
		cbor::push_float(out, std::fmod(y, 8.0f));
		cbor::push_float(out, std::fmod(z, 8.0f));
	}
}

Json StarData::to_json()
{

	Json star_obj = {};

	star_obj["name"] = starName;
	star_obj["absmag"] = absoluteMagnitude;
	star_obj["spectral"] = spectralType;
	star_obj["color"] = colorIndex;
	star_obj["constl"] = constellation;

	// FIXME: temporary workaround until I get clang-format to be sane with initializer lists
	/* clang-format off */
	star_obj["sector"] = Json::array({
		int(std::floor(x)) / 8,
		int(std::floor(y)) / 8,
		int(std::floor(z)) / 8
	});

	star_obj["coords"] = Json::array({
		std::fmod(x, 8.0f),
		std::fmod(y, 8.0f),
		std::fmod(z, 8.0f)
	});
	/* clang-format on */

	return star_obj;
}

int parse_database(argh::parser &sourceFiles, bool outputBinary, bool compress)
{
	std::vector<uint8_t> output_accum;
	output_accum.reserve(1024);
	// unbounded CBOR array
	output_accum.push_back(static_cast<uint8_t>(CBORTag::Array) << 5 | 31);

	Json fileObject = Json::array();

	for (size_t idx = 1; idx < sourceFiles.size(); idx++) {
		auto &inputFile = sourceFiles[idx];
		try {
			CSVReader reader(inputFile);
			std::vector<StarData> data = parse_csv(inputFile, reader);

			for (auto star : data) {
				if (outputBinary) {
					star.to_cbor(output_accum);
				} else {
					fileObject.push_back(star.to_json());
				}
			}
		} catch (std::runtime_error &e) {
			std::cerr << "Error parsing csv file:" << std::endl;
			std::cerr << e.what() << std::endl;
			return 1;
		}
	}

	// CBOR 'break' tag
	output_accum.push_back(0xFF);

	if (outputBinary) {
		string_view data(
			reinterpret_cast<const char *>(output_accum.data()),
			output_accum.size());

		if (compress)
			// use compression level 6 - there's no noticable difference in size
			// between 6 and 12, and the latter takes 4x as long
			std::cout << lz4::CompressLZ4(data, 6);
		else
			std::cout << data;

	} else {
		std::cout << fileObject.dump() << std::endl;
	}

	std::flush(std::cout);
	return 0;
}

int run_validation(argh::parser &sourceFiles)
{
	std::string input = sourceFiles[1];

	auto inputStream = std::ifstream(input, std::ios::ate);
	size_t size = inputStream.tellg();
	inputStream.seekg(0, std::ios::beg);

	std::unique_ptr<char[]> fileData(new char[size]);
	inputStream.read(fileData.get(), size);
	inputStream.close();
	string_view data{ fileData.get(), size };

	std::string decompressed_data;
	// LZ4 magic number
	if (lz4::IsLZ4Format(data.data(), data.size())) {
		decompressed_data = lz4::DecompressLZ4(data);
		data = { decompressed_data.data(), decompressed_data.size() };
		fileData.reset();
	}

	Json inputJson;
	// CBOR magic number
	if (reinterpret_cast<const uint8_t *>(data.data())[0] == 0x9F) {
		try {
			inputJson = Json::from_cbor({ data.data(), data.size() });
			std::cout << inputJson.dump() << std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}

		return 0;
	} else {
		std::cout << "Invalid input file " << input << ": file is not valid CBOR data!" << std::endl;
		return 1;
	}
}

int main(int argc, const char **argv)
{
	std::string help_text =
		"This program parses the HYG database of stars and converts it into a listing of\n"
		"custom stars for Pioneer to use.\n\n"
		"USAGE:\n"
		"\thyg-database-parser [OPTIONS] [FILE...]\n\n"
		"OPTIONS:\n\n"
		"\t-h  --help         Display this help menu\n"
		"\t-o  [FILE]         Output file name. If not present, writes to stdout\n"
		"\t    --binary       Output compressed CBOR files (default)\n"
		"\t    --no-compress  Output uncompressed CBOR files\n"
		"\t    --validate     Interpret input files as CBOR and convert to equivalent\n"
		"\t                   JSON for round-trip validation\n";

	argh::parser args({ "-o" });
	args.parse(argv);

	if (args[{ "-h", "--help" }] || args.size() == 0) {
		std::cerr << help_text;
		return 0;
	}

	bool binary = args["--binary"];
	bool compress = !args["--no-compress"];
	bool validate = args["--validate"];
	std::string outputFile;

	auto stdout = std::cout.rdbuf();
	if (args("o") >> outputFile) {
		auto fbuf = new std::filebuf();
		fbuf->open(outputFile, std::ios::out);
		std::cout.rdbuf(fbuf);
	}

	if (validate)
		return run_validation(args);
	else
		return parse_database(args, binary, compress);
}
