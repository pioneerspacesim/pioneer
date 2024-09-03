// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FaceParts.h"
#include "FileSystem.h"
#include "JobQueue.h"
#include "Pi.h"
#include "SDLWrappers.h"
#include "utils.h"

namespace {
	static const int MAX_GENDERS = 6;
	static const int MAX_RACES = 16;
	static const int MAX_SPECIES = 10;
	static const Uint32 GENDER_SHIFT = 0;
	static const Uint32 GENDER_MASK = ((1u << MAX_GENDERS) - 1u) << GENDER_SHIFT;
	static const Uint32 RACE_SHIFT = GENDER_SHIFT + MAX_GENDERS;
	static const Uint32 RACE_MASK = ((1u << MAX_RACES) - 1u) << RACE_SHIFT;
	static const Uint32 SPECIES_SHIFT = RACE_SHIFT + MAX_RACES;
	static const Uint32 SPECIES_MASK = ((1u << MAX_SPECIES) - 1u) << SPECIES_SHIFT;

	// You can never have too many static_asserts, right?
	static_assert(((MAX_GENDERS + MAX_RACES + MAX_SPECIES) == 32), "unused bits in the face part selector");
	static_assert(((GENDER_MASK | RACE_MASK | SPECIES_MASK) == UINT32_MAX), "unused bits in the face part selector");
	static_assert(((GENDER_MASK & RACE_MASK) == 0u), "face part selector: overlap between gender and race mask");
	static_assert(((GENDER_MASK & SPECIES_MASK) == 0u), "face part selector: overlap between gender and species mask");
	static_assert(((RACE_MASK & SPECIES_MASK) == 0u), "face part selector: overlap between race and species mask");

	struct Part {
		Uint32 selector; // a bitmask indicating which species, races and genders can use this part
		SDLSurfacePtr part;

		Part() :
			selector(0u) {}
		Part(const Uint32 sel, SDLSurfacePtr im) :
			selector(sel),
			part(im) {}
	};

	struct SpeciesInfo {
		int num_races;
		int num_genders;

		SpeciesInfo(int nraces, int ngenders) :
			num_races(nraces),
			num_genders(ngenders) {}
	};

	struct ScanPartJob : public Job {
		ScanPartJob(std::vector<Part> &output, int species_idx, int race_idx, const std::string &path, const char *prefix) :
			output(output),
			species_idx(species_idx),
			race_idx(race_idx),
			path(path),
			prefix(prefix)
		{}

		virtual void OnRun() override;
		virtual void OnFinish() override
		{
			for (auto &part : cache)
				output.push_back(std::move(part));
		}

	protected:
		std::vector<Part> &output;
		std::vector<Part> cache;
		int species_idx;
		int race_idx;
		const std::string path;
		const std::string prefix;
	};

	struct ScanGenderedPartJob : public ScanPartJob {
		using ScanPartJob::ScanPartJob;
		virtual void OnRun() override;
	};

	class PartDb {
	public:
		std::vector<SpeciesInfo> species;

		std::vector<Part> heads;
		std::vector<Part> eyes;
		std::vector<Part> noses;
		std::vector<Part> mouths;
		std::vector<Part> hairstyles;
		std::vector<Part> accessories;
		std::vector<Part> clothes;
		std::vector<Part> armour;

		SDLSurfacePtr background_general;

		void Clear();
		void Scan();

	private:
		void ScanSpecies(const std::string &dir, int species_idx);
		void QueueScan(ScanPartJob *job)
		{
			Pi::GetApp()->GetAsyncStartupQueue()->Order(job);
		}
	};

	static Uint32 _make_selector(int species, int race, int gender)
	{
		assert(species < MAX_SPECIES);
		assert(race < MAX_RACES);
		assert(gender < MAX_GENDERS);

		Uint32 mask = 0u;
		if (species < 0) {
			mask |= SPECIES_MASK;
		} else {
			mask |= (1u << (species + SPECIES_SHIFT));
		}
		if (race < 0) {
			mask |= RACE_MASK;
		} else {
			mask |= (1u << (race + RACE_SHIFT));
		}
		if (gender < 0) {
			mask |= GENDER_MASK;
		} else {
			mask |= (1u << (gender + GENDER_SHIFT));
		}
		return mask;
	}

	static int _count_parts(const std::vector<Part> &parts, const Uint32 selector)
	{
		int count = 0;
		for (const auto &part : parts) {
			if ((selector & part.selector) == selector) ++count;
		}
		return count;
	}

	static SDL_Surface *_get_part(const std::vector<Part> &parts, const Uint32 selector, int index)
	{
		for (const auto &part : parts) {
			if ((selector & part.selector) == selector) {
				if (!index) {
					return part.part.Get();
				}
				--index;
			}
		}
		return nullptr;
	}

	static void _blit_image(SDL_Surface *target, SDL_Surface *source, int xoff, int yoff)
	{
		assert(target);
		if (!source) return;
		SDL_Rect destrec = { 0, 0, 0, 0 };
		// if the source is the full size, then ignore the offset
		if ((source->w == FaceParts::FACE_WIDTH) &&
			(source->h == FaceParts::FACE_HEIGHT)) {
			destrec.x = 0;
			destrec.y = 0;
		} else {
			destrec.x = ((FaceParts::FACE_WIDTH - source->w) / 2) + xoff;
			destrec.y = yoff;
		}
		SDL_BlitSurface(source, 0, target, &destrec);
	}

	static PartDb *s_partdb;
} // anonymous namespace

namespace fs = FileSystem;

void PartDb::Clear()
{
	species.clear();
	heads.clear();
	eyes.clear();
	noses.clear();
	mouths.clear();
	hairstyles.clear();
	accessories.clear();
	clothes.clear();
	armour.clear();
}

static const char BACKGROUND_GENERAL_PATH[] = "facegen/backgrounds/general.png";

void PartDb::Scan()
{
	PROFILE_SCOPED()
	Clear();

	background_general = LoadSurfaceFromFile(BACKGROUND_GENERAL_PATH);
	if (!background_general) {
		Output("Failed to load image %s\n", BACKGROUND_GENERAL_PATH);
	}

	int species_count = 0;
	const auto flags = fs::FileEnumerator::IncludeDirs | fs::FileEnumerator::ExcludeFiles;
	for (fs::FileEnumerator dirs(fs::gameDataFiles, "facegen", flags); !dirs.Finished(); dirs.Next()) {
		if (!starts_with(dirs.Current().GetName(), "species_"))
			continue;
		if (species_count >= MAX_SPECIES) {
			Output("FaceParts: reached the limit on the number of species\n");
			break;
		}
		ScanSpecies(dirs.Current().GetPath(), species_count);
		++species_count;
	}
}

void PartDb::ScanSpecies(const std::string &basedir, const int species_idx)
{
	PROFILE_SCOPED()
	int race_count = 0;
	const auto flags = fs::FileEnumerator::IncludeDirs | fs::FileEnumerator::ExcludeFiles;
	for (fs::FileEnumerator dirs(fs::gameDataFiles, basedir, flags); !dirs.Finished(); dirs.Next()) {
		const std::string &path = dirs.Current().GetPath();
		const std::string &name = dirs.Current().GetName();

		if (name == "accessories") {
			QueueScan(new ScanPartJob(this->accessories, species_idx, -1, path, "acc_"));
		} else if (name == "clothes") {
			QueueScan(new ScanGenderedPartJob(this->clothes, species_idx, -1, path, "cloth_"));
			QueueScan(new ScanPartJob(this->armour, species_idx, -1, path, "armour_"));
		} else if (starts_with(name, "race_")) {
			if (race_count >= MAX_RACES) {
				Output("FaceParts: reached the limit on the number of races\n");
				continue; // continue to ensure 'accessories' and 'clothes' dirs can still be scanned
			}
			const int race_idx = race_count++;

			QueueScan(new ScanGenderedPartJob(this->heads, species_idx, race_idx, fs::JoinPath(path, "head"), "head_"));
			QueueScan(new ScanGenderedPartJob(this->eyes, species_idx, race_idx, fs::JoinPath(path, "eyes"), "eyes_"));
			QueueScan(new ScanGenderedPartJob(this->noses, species_idx, race_idx, fs::JoinPath(path, "nose"), "nose_"));
			QueueScan(new ScanGenderedPartJob(this->mouths, species_idx, race_idx, fs::JoinPath(path, "mouth"), "mouth_"));
			QueueScan(new ScanGenderedPartJob(this->hairstyles, species_idx, race_idx, fs::JoinPath(path, "hair"), "hair_"));
		} else {
			Output("FaceParts: unknown directory '%s'\n", path.c_str());
		}
	}

	species.push_back(SpeciesInfo(race_count, 2)); // XXX currently we hardcode genders = 2
}

void ScanPartJob::OnRun()
{
	PROFILE_SCOPED()
	const Uint32 selector = _make_selector(species_idx, race_idx, -1);
	for (fs::FileEnumerator files(fs::gameDataFiles, path); !files.Finished(); files.Next()) {
		const std::string &name = files.Current().GetName();
		if (starts_with(name, prefix)) {
			SDLSurfacePtr im = LoadSurfaceFromFile(files.Current().GetPath());
			if (im) {
				cache.push_back(Part(selector, im));
			} else {
				Output("Failed to load image %s\n", files.Current().GetPath().c_str());
			}
		}
	}
}

void ScanGenderedPartJob::OnRun()
{
	PROFILE_SCOPED()
	const int prefix_len = prefix.size();
	for (fs::FileEnumerator files(fs::gameDataFiles, path); !files.Finished(); files.Next()) {
		const std::string &name = files.Current().GetName();
		if (starts_with(name, prefix)) {
			char *end = nullptr;
			int gender_idx = strtol(name.c_str() + prefix_len, &end, 10);
			Uint32 sel;
			// HACK -- attempt to recognise `foo_3.png' style names
			if (strcmp(end, ".png") == 0) {
				sel = _make_selector(species_idx, race_idx, -1);
			} else {
				if (gender_idx < 0 || gender_idx >= MAX_GENDERS) {
					Output("Gender out of range: %s\n", files.Current().GetPath().c_str());
					continue;
				}
				sel = _make_selector(species_idx, race_idx, gender_idx);
			}

			SDLSurfacePtr im = LoadSurfaceFromFile(files.Current().GetPath());
			if (im) {
				cache.push_back(Part(sel, im));
			} else {
				Output("Failed to load image %s\n", files.Current().GetPath().c_str());
			}
		}
	}
}

const int FaceParts::FACE_WIDTH = 295;
const int FaceParts::FACE_HEIGHT = 285;

void FaceParts::Init()
{
	PROFILE_SCOPED()
	s_partdb = new PartDb;
	s_partdb->Scan();
	Output("Face Generation source images loaded.\n");
}

void FaceParts::Uninit()
{
	delete s_partdb;
	s_partdb = nullptr;
}

int FaceParts::NumSpecies()
{
	return s_partdb->species.size();
}

int FaceParts::NumGenders(const int speciesIdx)
{
	assert(speciesIdx >= 0 && speciesIdx < NumSpecies());
	return s_partdb->species[speciesIdx].num_genders;
}

int FaceParts::NumRaces(const int speciesIdx)
{
	assert(speciesIdx >= 0 && speciesIdx < NumSpecies());
	return s_partdb->species[speciesIdx].num_races;
}

int FaceParts::NumHeads(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->heads, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumEyes(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->eyes, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumNoses(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->noses, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumMouths(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->mouths, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumHairstyles(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->hairstyles, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumClothes(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->clothes, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumAccessories(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->accessories, _make_selector(speciesIdx, raceIdx, genderIdx));
}

int FaceParts::NumArmour(const int speciesIdx, const int raceIdx, const int genderIdx)
{
	return _count_parts(s_partdb->armour, _make_selector(speciesIdx, raceIdx, genderIdx));
}

static void _pick(Random &rng, int &inout_value, const int limit)
{
	assert(limit > 0);
	// we always run the RNG, even if the result is not needed, because that way
	// the output (index) for a particular component should be fixed for a given seed,
	// independent of changes to other components
	const Uint32 rng_value = (rng.Int32() % limit);
	if (inout_value < 0) {
		inout_value = rng_value;
		assert(inout_value >= 0 && inout_value < limit);
	} else {
		inout_value = (inout_value % limit);
	}
}

void FaceParts::PickFaceParts(FaceDescriptor &inout_face, const Uint32 seed)
{
	PROFILE_SCOPED()
	Random rand(seed);

	_pick(rand, inout_face.species, NumSpecies());
	_pick(rand, inout_face.race, NumRaces(inout_face.species));
	_pick(rand, inout_face.gender, NumGenders(inout_face.species));

	const Uint32 selector = _make_selector(inout_face.species, inout_face.race, inout_face.gender);

	_pick(rand, inout_face.head, _count_parts(s_partdb->heads, selector));
	_pick(rand, inout_face.eyes, _count_parts(s_partdb->eyes, selector));
	_pick(rand, inout_face.nose, _count_parts(s_partdb->noses, selector));
	_pick(rand, inout_face.mouth, _count_parts(s_partdb->mouths, selector));
	_pick(rand, inout_face.hairstyle, _count_parts(s_partdb->hairstyles, selector));

	const bool has_accessories = (rand.Int32() & 1 || inout_face.accessories >= 0);
	_pick(rand, inout_face.accessories, _count_parts(s_partdb->accessories, selector));
	if (!has_accessories) {
		inout_face.accessories = 0;
	}

	_pick(rand, inout_face.clothes, _count_parts(s_partdb->clothes, selector));
	_pick(rand, inout_face.armour, _count_parts(s_partdb->armour, selector));
}

void FaceParts::BuildFaceImage(SDL_Surface *faceIm, const FaceDescriptor &face)
{
	PROFILE_SCOPED()
	const Uint32 selector = _make_selector(face.species, face.race, face.gender);

	_blit_image(faceIm, s_partdb->background_general.Get(), 0, 0);
	_blit_image(faceIm, _get_part(s_partdb->heads, selector, face.head), 0, 0);
	if (!face.armour) {
		_blit_image(faceIm, _get_part(s_partdb->clothes, selector, face.clothes), 0, 135);
	}
	_blit_image(faceIm, _get_part(s_partdb->eyes, selector, face.eyes), 0, 41);
	_blit_image(faceIm, _get_part(s_partdb->noses, selector, face.nose), 1, 89);
	_blit_image(faceIm, _get_part(s_partdb->mouths, selector, face.mouth), 0, 155);
	if (!face.armour) {
		_blit_image(faceIm, _get_part(s_partdb->accessories, selector, face.accessories), 0, 0);
		_blit_image(faceIm, _get_part(s_partdb->hairstyles, selector, face.hairstyle), 0, 0);
	} else {
		_blit_image(faceIm, _get_part(s_partdb->armour, selector, face.armour), 0, 0);
	}
}
