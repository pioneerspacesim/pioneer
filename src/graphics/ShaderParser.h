// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Texture.h"
#include "graphics/Types.h"
#include "core/ConfigParser.h"

namespace Graphics {

	namespace ShaderParser {

		struct TextureInfo {
			Graphics::TextureType type;
			std::string bindName;
			std::string name;
			uint32_t binding;
		};

		struct BufferInfo {
			std::string name;
			uint32_t binding;
		};

		struct PushConstantInfo {
			Graphics::ConstantDataFormat type;
			std::string name;
			uint32_t binding;
		};

		struct ShaderInfo {
			std::string name;
			std::string vertexPath;
			std::string fragmentPath;

			std::vector<TextureInfo> textureBindings;
			std::vector<BufferInfo> bufferBindings;
			std::vector<PushConstantInfo> pushConstantBindings;
		};

		struct ParserContext : Config::Parser::Context {
			using Parser = Config::Parser;
			using Token = Config::Token;

			ParserContext(ShaderInfo *shaderData) :
				m_shaderData(shaderData)
			{}
			~ParserContext() = default;

			Result operator()(Parser *parser, const Token &tok) final;

		private:
			Result parseTexture(Parser *parser);
			Result parseBuffer(Parser *parser);
			Result parsePushConstant(Parser *parser);

			ShaderInfo *m_shaderData;
			uint32_t m_nextTextureBinding = 0;
			uint32_t m_nextBufferBinding = 0;
			uint32_t m_nextPushConstantBinding = 0;
		};

	} // namespace ShaderParser

} // namespace Graphics
