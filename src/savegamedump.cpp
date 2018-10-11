// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Json.h"
#include "FileSystem.h"
#include "GZipFormat.h"

int main(int argc, const char **argv) {
    if (argc < 2 || argc > 3) {
        printf(
            "savegamedump - Dump saved games to JSON for easy inspection.\n"
            "All paths are relative to the pioneer data folder.\n"
            "USAGE: savegamedump <input> [output]\n"
        );
        return 1;
    }
    std::string filename = argv[1];
    std::string outname = argc > 2 ? argv[2] : filename + ".json";

    auto fileinfo = FileSystem::userFiles.Lookup(filename);
    if (!fileinfo.Exists()) {
        printf("Input file %s could not be found.\n", filename.c_str());
        printf("%s\n", fileinfo.GetPath().c_str());
        return 1;
    }

    auto file = FileSystem::userFiles.ReadFile(filename);
	if (!file) {
        printf( "Could not open file %s.\n", filename.c_str());
        return 1;
    }

	const auto compressed_data = file->AsByteRange();
    Json rootNode;
	try {
		const std::string plain_data = gzip::DecompressDeflateOrGZip(reinterpret_cast<const unsigned char*>(compressed_data.begin), compressed_data.Size());
		try {
			// Allow loading files in JSON format as well as CBOR
			if (plain_data[0] == '{') rootNode = Json::parse(plain_data);
			else rootNode = Json::from_cbor(plain_data);
		} catch (Json::parse_error &e) {
			printf("Saved game is not a valid JSON object: %s.\n", e.what());
			return 2;
		}

		if (!rootNode.is_object()) {
            printf("Saved game's root is not a JSON object.\n");
            return 2;
        }
	} catch (gzip::DecompressionFailedException) {
		printf("Decompressing saved data failed - saved game is corrupt.\n");
        return 3;
	}

    auto outFile = FileSystem::userFiles.OpenWriteStream(outname);
    if (!outFile) {
        printf("Could not open output file %s.\n", outname.c_str());
        return 1;
    }

    fputs(rootNode.dump(2).c_str(), outFile);
    fclose(outFile);

    return 0;
}
