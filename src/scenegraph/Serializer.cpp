// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Serializer.h"
#include "GameSaveError.h"

namespace Serializer {

	Reader Reader::RdSection(const std::string &section_label_expected)
	{
		if (section_label_expected != String()) {
			throw SavedGameCorruptException();
		}
		Reader section = Reader(Blob());
		section.SetStreamVersion(StreamVersion());
		return section;
	}

} /* end namespace Serializer */
