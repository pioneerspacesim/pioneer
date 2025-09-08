// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Parser.h"
#include "FileSystem.h"
#include "MathUtil.h"
#include "StringF.h"
#include "StringRange.h"
#include "core/ConfigParser.h"
#include "core/Log.h"
#include "graphics/Types.h"
#include "profiler/Profiler.h"
#include "scenegraph/LoaderDefinitions.h"

#include <sstream>

enum Tok : uint8_t {
	LBrace = '{',
	RBrace = '}',
	LBracket = '[',
	RBracket = ']',
};

namespace SceneGraph {

	bool LodSortPredicate(const LodDefinition &a, const LodDefinition &b)
	{
		return a.pixelSize < b.pixelSize;
	}

	Parser::Parser(FileSystem::FileSource &fs, const std::string &filename, const std::string &path) :
		m_isMaterial(false),
		m_curMat(0),
		m_model(0),
		m_path(path)
	{
		RefCountedPtr<FileSystem::FileData> data = fs.ReadFile(filename);
		if (!data) throw ParseError("Could not open");
		m_file = data;
	}

	void Parser::Parse(ModelDefinition *m)
	{
		PROFILE_SCOPED()
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
		if (m->matDefs.empty()) {
			MaterialDefinition &mat = m->matDefs.emplace_back();
			mat.name = "Default";
			mat.shader = "multi";
		}

		//sort lods by feature size
		std::sort(m->lodDefs.begin(), m->lodDefs.end(), LodSortPredicate);
	}

	bool Parser::isComment(const std::string &s)
	{
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
		if (!(ss >> out)) throw ParseError(stringf("Expected %0, got nothing", what));
		if (isComment(out)) throw ParseError(stringf("Expected %0, got comment", what));
		return true;
	}

	bool Parser::checkTexture(std::stringstream &ss, std::string &out)
	{
		checkString(ss, out, "file name");
		//add newmodels/some_model/ to path
		out = FileSystem::NormalisePath(FileSystem::JoinPath(m_path, out));
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
		color.r = Clamp(r, 0.f, 1.f) * 255;
		color.g = Clamp(g, 0.f, 1.f) * 255;
		color.b = Clamp(b, 0.f, 1.f) * 255;
		color.a = 255; //alpha comes from opacity statement
		return true;
	}

	bool Parser::parseLine(const std::string &line)
	{
		PROFILE_SCOPED()
		using std::string;
		using std::stringstream;
		stringstream ss(stringstream::in | stringstream::out);
		ss.str(line);
		if (ss.fail()) throw ParseError("Stringstream failure");
		string token;
		if (ss >> token) {
			//line contains something
			if (isComment(token))
				return true; //skip comments
			if (match(token, "material")) {
				//beginning of a new material definition,
				//expect a name and then parameters on following lines
				m_isMaterial = true;
				string matname;
				checkMaterialName(ss, matname);
				m_curMat = &m_model->matDefs.emplace_back();
				m_curMat->name = matname;
				m_curMat->shader = "multi";
				return true;
			} else if (match(token, "lod")) {
				endMaterial();
				float featuresize;
				if (!(ss >> featuresize))
					throw ParseError("Detail level must specify a pixel size");
				if (is_zero_general(featuresize))
					throw ParseError("Detail level pixel size must be greater than 0");
				m_model->lodDefs.push_back(LodDefinition(featuresize));
				return true;
			} else if (match(token, "mesh")) {
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
			} else if (match(token, "collision")) {
				//collision mesh definitions contain also only a filename
				endMaterial();
				string cmeshname;
				checkMesh(ss, cmeshname);
				m_model->collisionDefs.push_back(cmeshname);
				return true;
			} else if (match(token, "anim")) {
				//anims should only affect the previously defined mesh but eh
				if (m_isMaterial || m_model->lodDefs.empty() || m_model->lodDefs.back().meshNames.empty())
					throw ParseError("Animation definition must come after a mesh definition");
				std::string animName;
				double startFrame;
				double endFrame;
				bool loopMode = false;
				std::string loop;
				checkString(ss, animName, "animation name");
				if (!(ss >> startFrame))
					throw ParseError("Animation start frame not defined");
				if (!(ss >> endFrame))
					throw ParseError("Animation end frame not defined");
				if (ss >> loop && match(loop, "loop"))
					loopMode = true;
				if (startFrame < 0 || endFrame < startFrame)
					throw ParseError("Animation start/end frames seem wrong");
				m_model->animDefs.push_back(AnimDefinition(animName, startFrame, endFrame, loopMode));
				return true;
			} else if (match(token, "bound")) {
				std::string kind, bound_name, start, end;
				double r;
				if(!(ss >> kind && ss >> bound_name && ss >> start && ss >> end && ss >> r)) {
					throw ParseError("Malformed boundary");
				}
				if(match(kind, "capsule")) {
					m_model->boundsDefs.push_back(BoundDefinition::create_capsule(bound_name, start, end, r));
				}
				else {
					throw ParseError("Unknown boundary kind");
				}
				return true;
			} else {
				if (m_isMaterial) {
					//material definition in progress, check known parameters
					string texname;
					if (match(token, "tex_diff") && checkTexture(ss, texname)) {
						m_curMat->textureBinds.push_back({ "diffuse", texname });
						return true;
					} else if (match(token, "tex_spec") && checkTexture(ss, texname)) {
						m_curMat->textureBinds.push_back({ "specular", texname });
						return true;
					} else if (match(token, "tex_glow") && checkTexture(ss, texname)) {
						m_curMat->textureBinds.push_back({ "glow", texname });
						return true;
					} else if (match(token, "tex_ambi") && checkTexture(ss, texname)) {
						m_curMat->textureBinds.push_back({ "ambient", texname });
						return true;
					} else if (match(token, "tex_norm") && checkTexture(ss, texname)) {
						m_curMat->textureBinds.push_back({ "normal", texname });
						return true;
					} else if (match(token, "diffuse"))
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

						if (m_curMat->opacity < 100) {
							m_curMat->renderState.blendMode = Graphics::BLEND_ALPHA;
							m_curMat->renderState.depthWrite = false;
						}
						return true;
					} else if (match(token, "alpha_test")) {
						m_curMat->alpha_test = true;
						return true;
					} else if (match(token, "unlit")) {
						m_curMat->unlit = true;
						return true;
					} else if (match(token, "use_patterns")) {
						m_curMat->use_patterns = true;
						return true;
					} else
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


	// ============================================================================

	using Token = Config::Token;

	// Helper function to parse an <r> <g> <b> color triplet
	static Config::Parser::Result parseColor(Color &out, Config::Parser *parser)
	{
		Token r, g, b;
		bool valid =
			parser->Acquire(Token::Number, &r) &&
			parser->Acquire(Token::Number, &g) &&
			parser->Acquire(Token::Number, &b);

		if (!valid)
			return Config::Parser::Result::ParseFailure;

		out = Color4f(r.value, g.value, b.value);
		return Config::Parser::Result::Parsed;
	}

	// Helper function to parse a clamped float value
	static Config::Parser::Result parseFloat(float &out, Config::Parser *parser, float min = 0, float max = FLT_MAX)
	{
		Token val;
		if (!parser->Acquire(Token::Number, &val))
			return Config::Parser::Result::ParseFailure;

		out = Clamp(float(val.value), min, max);
		return Config::Parser::Result::Parsed;
	}

	static Config::Parser::Result parseOnOff(bool &out, Config::Parser *parser)
	{
		Token onOff;
		if (!parser->Acquire(Token::Identifier, &onOff))
			return Config::Parser::Result::ParseFailure;

		if (onOff.isKeyword("on")) {
			out = true;
			return Config::Parser::Result::Parsed;
		} else if (onOff.isKeyword("off")) {
			out = false;
			return Config::Parser::Result::Parsed;
		}

		Log::Warning("{} Expected one of 'on' or 'off', got '{}'",
			parser->MakeLineInfo(), onOff.range);
		return Config::Parser::Result::ParseFailure;
	}

	// ============================================================================

	struct ParseRenderStateCtx : Config::Parser::Context {
		using Parser = Config::Parser;

		ParseRenderStateCtx(Graphics::RenderStateDesc *render_state) :
			render_state(render_state) {}

		Result operator()(Config::Parser *parser, const Token &tok) final
		{
			if (tok.isKeyword("blend")) {
				Token state;
				if (!parser->Acquire(Token::Identifier, &state))
					return Result::ParseFailure;

				if (state.isKeyword("solid"))
					render_state->blendMode = Graphics::BLEND_SOLID;
				else if (state.isKeyword("additive"))
					render_state->blendMode = Graphics::BLEND_ADDITIVE;
				else if (state.isKeyword("alpha"))
					render_state->blendMode = Graphics::BLEND_ALPHA;
				else if (state.isKeyword("alpha_one"))
					render_state->blendMode = Graphics::BLEND_ALPHA_ONE;
				else if (state.isKeyword("alpha_premult"))
					render_state->blendMode = Graphics::BLEND_ALPHA_PREMULT;
				else {
					Log::Warning("{} Unknown blend mode '{}'", parser->MakeLineInfo(), state.range);
					return Result::ParseFailure;
				}

				return Result::Parsed;
			}

			if (tok.isKeyword("cull")) {
				Token state;
				if (!parser->Acquire(Token::Identifier, &state))
					return Result::ParseFailure;

				if (state.isKeyword("front"))
					render_state->cullMode = Graphics::CULL_FRONT;
				else if (state.isKeyword("back"))
					render_state->cullMode = Graphics::CULL_BACK;
				else if (state.isKeyword("none"))
					render_state->cullMode = Graphics::CULL_NONE;
				else {
					Log::Warning("{} Unknown cull mode '{}'", parser->MakeLineInfo(), state.range);
					return Result::ParseFailure;
				}
			}

			if (tok.isKeyword("depth_test")) {
				return parseOnOff(render_state->depthTest, parser);
			} else if (tok.isKeyword("depth_write")) {
				return parseOnOff(render_state->depthWrite, parser);
			} else if (tok.isKeyword("scissor_test")) {
				return parseOnOff(render_state->scissorTest, parser);
			}

			if (tok.id == Tok::RBrace) {
				parser->PopContext(this);
				return Result::Parsed;
			}

			Log::Warning("{} Unknown token '{}' while parsing material render state definition.",
				parser->MakeLineInfo(), tok.range);
			return Result::DidNotMatch;
		}

		Graphics::RenderStateDesc *render_state;
	};

	// ============================================================================

	struct ParseMaterialCtx : Config::Parser::Context {
		ParseMaterialCtx(MaterialDefinition *m, const std::string &dirname) :
			mat(m), dirname(dirname) {}

		Result operator()(Config::Parser *parser, const Token &tok) final
		{
			if (tok.isKeyword("shader")) {
				Token shader;

				if (!parser->Acquire(Token::String, &shader))
					return Result::ParseFailure;

				mat->shader = shader.toString();
				return Result::Parsed;
			}

			if (tok.isKeyword("diffuse")) {
				return parseColor(mat->diffuse, parser);
			} else if (tok.isKeyword("specular")) {
				return parseColor(mat->specular, parser);
			} else if (tok.isKeyword("ambient")) {
				return parseColor(mat->ambient, parser);
			} else if (tok.isKeyword("emissive")) {
				return parseColor(mat->emissive, parser);
			} else if (tok.isKeyword("shininess")) {
				return parseFloat(mat->shininess, parser, 0, 128);
			} else if (tok.isKeyword("opacity")) {
				return parseFloat(mat->opacity, parser, 0, 100);
			} else if (tok.isKeyword("use_patterns")) {
				mat->use_patterns = true;
				return Result::Parsed;
			} else if (tok.isKeyword("unlit")) {
				mat->unlit = true;
				return Result::Parsed;
			}

			if (tok.isKeyword("texture")) {
				Token binding, path;

				if (!parser->Acquire(Token::Identifier, &binding) || !parser->Acquire(Token::String, &path))
					return Result::ParseFailure;

				std::string texpath = FileSystem::NormalisePath(FileSystem::JoinPath(dirname, std::string(path.contents())));
				mat->textureBinds.emplace_back(binding.contents(), texpath);
				return Result::Parsed;
			}

			if (tok.isKeyword("render_state")) {
				if (!parser->Symbol(Tok::LBrace))
					return Result::ParseFailure;

				parser->PushContext(new ParseRenderStateCtx(&mat->renderState));
				return Result::Parsed;
			}

			if (tok.isKeyword("alpha_test")) {
				mat->alpha_test = true;
				return Result::Parsed;
			}

			if (tok.isSymbol(Tok::RBrace)) {
				return parser->PopContext(this);
			}

			Log::Warning("{} unexpected token '{}' while parsing material definition.",
				parser->MakeLineInfo(), tok.range);
			return Result::DidNotMatch;
		}

		MaterialDefinition *mat;
		const std::string &dirname;
	};

	// ============================================================================

	struct ParseLodCtx : Config::Parser::Context {
		ParseLodCtx(LodDefinition *node, const std::string &dirname) :
			node(node), dirname(dirname) {}

		Result operator()(Config::Parser *parser, const Token &tok) final
		{
			if (tok.isKeyword("mesh")) {
				Token meshName;

				if (!parser->Acquire(Token::String, &meshName))
					return Result::ParseFailure;

				std::string path = FileSystem::NormalisePath(FileSystem::JoinPath(dirname, std::string(meshName.contents())));
				node->meshNames.emplace_back(path);
				return Result::Parsed;
			}

			if (tok.isSymbol(Tok::RBrace)) {
				return parser->PopContext(this);
			}

			Log::Warning("{} unexpected token '{}' while parsing LOD definition.",
				parser->MakeLineInfo(), tok.range);
			return Result::DidNotMatch;
		}

		LodDefinition *node;
		const std::string &dirname;
	};

	// ============================================================================

	struct TopLevelCtx : Config::Parser::Context {
		TopLevelCtx(ModelDefinition *m, const std::string &dirname) :
			model(m), dirname(dirname) {}

		Result operator()(Config::Parser *parser, const Token &tok) final
		{
			// Handle the version keyword
			if (tok.isKeyword("version")) {
				Token version;
				if (!parser->Acquire(Token::Number, &version))
					return Result::ParseFailure;

				if (version.value != 2) {
					Log::Warning("{} Material format version {} not recognized.", parser->MakeLineInfo(), version.value);
					return Result::ParseFailure;
				}

				return Result::Parsed;
			}

			if (tok.isKeyword("material")) {
				Token name;

				if (!parser->Acquire(Token::String, &name) || !parser->Symbol(Tok::LBrace))
					return Result::ParseFailure;

				if (name.contents().empty()) {
					Log::Warning("{} Material cannot have an empty name.", parser->MakeLineInfo());
					return Result::ParseFailure;
				}

				for (const auto &matDef : model->matDefs) {
					if (matDef.name.compare(name.contents()) == 0) {
						Log::Warning("{} Material '{}' is already defined.");
						return Result::ParseFailure;
					}
				}

				MaterialDefinition &mat = model->matDefs.emplace_back();
				mat.name = name.toString();
				// Default to multi if no shader is explicitly selected.
				mat.shader = "multi";

				return parser->PushContext(new ParseMaterialCtx(&model->matDefs.back(), dirname));
			}

			if (tok.isKeyword("lod")) {
				Token featuresize;

				if (!parser->Acquire(Token::Number, &featuresize) || !parser->Symbol(Tok::LBrace))
					return Result::ParseFailure;

				if (is_zero_general(featuresize.value)) {
					Log::Warning("{} LOD detail level must be greater than 0");
					return Result::ParseFailure;
				}

				model->lodDefs.push_back(LodDefinition(featuresize.value));
				parser->PushContext(new ParseLodCtx(&model->lodDefs.back(), dirname));

				return Result::Parsed;
			}

			if (tok.isKeyword("collision")) {
				Token collMesh;

				if (!parser->Acquire(Token::String, &collMesh))
					return Result::ParseFailure;

				std::string path = FileSystem::NormalisePath(FileSystem::JoinPath(dirname, std::string(collMesh.contents())));
				model->collisionDefs.emplace_back(path);
				return Result::Parsed;
			}

			if (tok.isKeyword("anim")) {
				Token name, fStart, fEnd;

				bool valid =
					parser->Acquire(Token::String, &name) &&
					parser->Acquire(Token::Number, &fStart) &&
					parser->Acquire(Token::Number, &fEnd);

				if (!valid)
					return Result::ParseFailure;

				if (fStart.value < 0 || fEnd.value < fStart.value) {
					Log::Warning("{}: Animation frames for anim '{}' seem wrong",
						parser->MakeLineInfo(), name.contents());
				}

				bool loop = parser->state.peek().isKeyword("loop");
				if (loop)
					parser->Advance();

				model->animDefs.emplace_back(name.toString(), fStart.value, fEnd.value, loop);
				return Result::Parsed;
			}

			if (tok.isKeyword("bound")) {
				Token type, name, tag1, tag2, radius;

				bool valid =
					parser->Acquire(Token::Identifier, &type) &&
					parser->Acquire(Token::Identifier, &name) &&
					parser->Acquire(Token::String, &tag1) &&
					parser->Acquire(Token::String, &tag2) &&
					parser->Acquire(Token::Number, &radius);

				if (!valid)
					return Result::ParseFailure;

				if (!type.isKeyword("capsule")) {
					Log::Warning("{} Unknown bound type '{}'", parser->MakeLineInfo(), type.range);
					return Result::ParseFailure;
				}

				model->boundsDefs.push_back(BoundDefinition::create_capsule(name.toString(), tag1.toString(), tag2.toString(), radius.value));
				return Result::Parsed;
			}

			Log::Warning("{} unexpected token '{}' in model file.",
				parser->MakeLineInfo(), tok.range);
			return Result::DidNotMatch;
		}

		ModelDefinition *model;
		const std::string &dirname;
	};

	// ============================================================================

	ParserV2::ParserV2()
	{
	}

	bool ParserV2::Parse(FileSystem::FileData &file, ModelDefinition *outModel)
	{
		const std::string &dirname = file.GetInfo().GetDir();
		Config::Parser parser { new TopLevelCtx(outModel, dirname), '#' };

		parser.Init(file.GetInfo().GetName(), file.AsStringView());

		Config::Parser::Result res = parser.Parse();

		if (outModel->lodDefs.empty() || outModel->lodDefs.back().meshNames.empty()) {
			Log::Warning("{}: no meshes defined!", file.GetInfo().GetName());
			return false;
		}

		//model without materials is not very useful, but not fatal - add white default mat
		if (outModel->matDefs.empty()) {
			Log::Warning("{}: no materials defined!", file.GetInfo().GetName());
			MaterialDefinition &mat = outModel->matDefs.emplace_back();

			mat.name = "Default";
			mat.shader = "multi";
		}

		//sort lods by feature size
		std::sort(outModel->lodDefs.begin(), outModel->lodDefs.end(), LodSortPredicate);

		return res == Config::Parser::Result::Parsed;
	}


} // namespace SceneGraph
