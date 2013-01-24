// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Parser.h"
#include "FileSystem.h"
#include "StringF.h"
#include "StringRange.h"
#include <sstream>

namespace SceneGraph {

bool LodSortPredicate(const LodDefinition &a, const LodDefinition &b)
{
	return a.pixelSize < b.pixelSize;
}

Parser::Parser(FileSystem::FileSource &fs, const std::string &filename, const std::string &path)
: m_isMaterial(false)
, m_curMat(0)
, m_model(0)
, m_path(path)
{
	RefCountedPtr<FileSystem::FileData> data = fs.ReadFile(filename);
	if (!data) throw ParseError("Could not open");
	m_file = data;
}

void Parser::Parse(ModelDefinition *m)
{
	StringRange buffer = m_file->AsStringRange();
	buffer = buffer.StripUTF8BOM();

	m_model = m;
	int lineno = 0;
	while (!buffer.Empty()) {
		lineno++;
		StringRange line = buffer.ReadLine();
		try {
			if (!parseLine(line.ToString()))
				throw ParseError("Mystery fail");
		} catch (ParseError &err) {
			std::stringstream ss;
			ss << "Error parsing line " << lineno << ":" << std::endl;
			ss << line.ToString() << std::endl;
			ss << err.what();
			throw ParseError(ss.str());
		}
	}

	if (m->lodDefs.empty() || m->lodDefs.back().meshNames.empty())
		throw ParseError("No meshes defined");

	//model without materials is not very useful, but not fatal - add white default mat
	if (m->matDefs.empty())
		m->matDefs.push_back(MaterialDefinition("Default"));

	//sort lods by feature size
	std::sort(m->lodDefs.begin(), m->lodDefs.end(), LodSortPredicate);
}

bool Parser::isComment(const std::string &s) {
	assert(!s.empty());
	return (s[0] == '#');
}

//check if string matches completely
bool Parser::match(const std::string &s, const std::string &what)
{
	return (s.compare(what) == 0);
}

//check for a string, but don't accept comments
bool Parser::checkString(std::stringstream &ss, std::string &out, const std::string &what)
{
	if (ss >> out == 0) throw ParseError(stringf("Expected %0, got nothing", what));
	if (isComment(out)) throw ParseError(stringf("Expected %0, got comment", what));
	return true;
}

bool Parser::checkTexture(std::stringstream &ss, std::string &out)
{
	checkString(ss, out, "file name");
	//add newmodels/some_model/ to path
	out = FileSystem::JoinPathBelow(m_path, out);
	return true;
}

inline bool Parser::checkMesh(std::stringstream &ss, std::string &out)
{
	return checkTexture(ss, out);
}

inline bool Parser::checkMaterialName(std::stringstream &ss, std::string &out)
{
	return checkString(ss, out, "material name");
}

bool Parser::checkColor(std::stringstream &ss, Color &color)
{
	float r, g, b;
	ss >> r >> g >> b;
	color.r = Clamp(r, 0.f, 1.f);
	color.g = Clamp(g, 0.f, 1.f);
	color.b = Clamp(b, 0.f, 1.f);
	color.a = 1.f; //alpha comes from opacity statement
	return true;
}

bool Parser::parseLine(const std::string &line)
{
	using std::stringstream;
	using std::string;
	stringstream ss(stringstream::in | stringstream::out);
	ss.str(line);
	if (ss.fail()) throw ParseError("Stringstream failure");
	string token;
	if ((ss >> token) != 0) {
		//line contains something
		if (isComment(token))
			return true; //skip comments
		if (match(token, "material")) {
			//beginning of a new material definition,
			//expect a name and then parameters on following lines
			m_isMaterial = true;
			string matname;
			checkMaterialName(ss, matname);
			m_model->matDefs.push_back(MaterialDefinition(matname));
			m_curMat = &m_model->matDefs.back();
			return true;
		} else if(match(token, "lod")) {
			endMaterial();
			float featuresize;
			if (ss >> featuresize == 0)
				throw ParseError("Detail level must specify a pixel size");
			if (is_zero_general(featuresize))
				throw ParseError("Detail level pixel size must be greater than 0");
			m_model->lodDefs.push_back(LodDefinition(featuresize));
			return true;
		} else if(match(token, "mesh")) {
			//mesh definitionss only contain a filename
			endMaterial();
			string meshname;
			checkMesh(ss, meshname);
			//model might not have specified lods at all.
			if (m_model->lodDefs.empty()) {
				m_model->lodDefs.push_back(LodDefinition(100.f));
			}
			m_model->lodDefs.back().meshNames.push_back(meshname);
			return true;
		} else if(match(token, "collision")) {
			//collision mesh definitions contain also only a filename
			endMaterial();
			string cmeshname;
			checkMesh(ss, cmeshname);
			m_model->collisionDefs.push_back(cmeshname);
			return true;
		} else if(match(token, "anim")) {
			//anims should only affect the previously defined mesh but eh
			if (m_isMaterial || m_model->lodDefs.empty() || m_model->lodDefs.back().meshNames.empty())
				throw ParseError("Animation definition must come after a mesh definition");
			std::string animName;
			double startFrame;
			double endFrame;
			bool loopMode = false;
			std::string loop;
			checkString(ss, animName, "animation name");
			if (ss >> startFrame == 0)
				throw ParseError("Animation start frame not defined");
			if (ss >> endFrame == 0)
				throw ParseError("Animation end frame not defined");
			if (ss >> loop && match(loop, "loop"))
				loopMode = true;
			if (startFrame < 0 || endFrame < startFrame)
				throw ParseError("Animation start/end frames seem wrong");
			m_model->animDefs.push_back(AnimDefinition(animName, startFrame, endFrame, loopMode));
			return true;
		} else {
			if (m_isMaterial) {
				//material definition in progress, check known parameters
				if (match(token, "tex_diff"))
					return checkTexture(ss, m_curMat->tex_diff);
				else if (match(token, "tex_spec"))
					return checkTexture(ss, m_curMat->tex_spec);
				else if (match(token, "tex_glow"))
					return checkTexture(ss, m_curMat->tex_glow);
				else if (match(token, "diffuse"))
					return checkColor(ss, m_curMat->diffuse);
				else if (match(token, "specular"))
					return checkColor(ss, m_curMat->specular);
				else if (match(token, "ambient"))
					return checkColor(ss, m_curMat->ambient);
				else if (match(token, "emissive"))
					return checkColor(ss, m_curMat->emissive);
				else if (match(token, "shininess")) {
					int shininess;
					ss >> shininess;
					m_curMat->shininess = Clamp(shininess, 0, 128);
					return true;
				} else if (match(token, "opacity")) {
					int opacity;
					ss >> opacity;
					m_curMat->opacity = Clamp(opacity, 0, 100);
					return true;
				} else if (match(token, "alpha_test")) {
					m_curMat->alpha_test = true;
					return true;
				} else if (match(token, "two_sided")) {
					m_curMat->two_sided = true;
					return true;
				} else if (match(token, "unlit")) {
					m_curMat->unlit = true;
					return true;
				} else if (match(token, "use_patterns")) {
					m_curMat->use_pattern = true;
					return true;
				}
				else
					throw ParseError("Unknown instruction");
			}
			throw ParseError("Unknown instruction");
		}
	} else {
		//empty line, skip
		return true;
	}
}

void Parser::endMaterial()
{
	m_isMaterial = false;
	m_curMat = 0;
}

}
