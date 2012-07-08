#ifndef _NEWMODEL_PARSER_H_
#define _NEWMODEL_PARSER_H_
/*
 * Newmodel .model config file parser. It's not the smartest thing ever.
 */
#include "Loader.h"
#include <fstream>

namespace Newmodel {

class Parser
{
public:
	Parser(const std::string &filename, const std::string &path);
	~Parser();

	void Parse(ModelDefinition *m);

private:
	bool m_isMaterial;
	MaterialDefinition *m_curMat;
	ModelDefinition *m_model;
	std::ifstream m_file;
	std::string m_path;

	bool isComment(const std::string &s);
	bool match(const std::string &s, const std::string &what);
	bool checkString(std::stringstream &ss, std::string &out, const std::string &what);
	bool checkTexture(std::stringstream &ss, std::string &out);
	inline bool checkMesh(std::stringstream &ss, std::string &out);
	inline bool checkMaterialName(std::stringstream &ss, std::string &out);
	bool checkColor(std::stringstream &ss, Color &color);
	bool parseLine(const std::string &line);
};

}

#endif
