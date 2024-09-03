// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_PARSER_H_
#define _SCENEGRAPH_PARSER_H_
/*
 * Newmodel .model config file parser.
 * It's pretty bad, someone please redesign.
 */
#include "FileSystem.h"
#include "Loader.h"
#include <stdexcept>

namespace SceneGraph {

	struct ParseError : public std::runtime_error {
		ParseError(const std::string &str) :
			std::runtime_error(str.c_str()) {}
	};

	class Parser {
	public:
		Parser(FileSystem::FileSource &, const std::string &filename, const std::string &path);

		void Parse(ModelDefinition *m);

	private:
		bool m_isMaterial;
		MaterialDefinition *m_curMat;
		ModelDefinition *m_model;
		RefCountedPtr<FileSystem::FileData> m_file;
		std::string m_path;

		bool checkColor(std::stringstream &ss, Color &color);
		bool checkString(std::stringstream &ss, std::string &out, const std::string &what);
		bool checkTexture(std::stringstream &ss, std::string &out);
		bool isComment(const std::string &s);
		bool match(const std::string &s, const std::string &what);
		bool parseLine(const std::string &line);
		inline bool checkMaterialName(std::stringstream &ss, std::string &out);
		inline bool checkMesh(std::stringstream &ss, std::string &out);
		void endMaterial();
	};

} // namespace SceneGraph

#endif
