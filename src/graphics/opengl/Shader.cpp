// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shader.h"
#include "FileSystem.h"
#include "Program.h"
#include "StringF.h"
#include "StringRange.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/ShaderParser.h"
#include "graphics/Types.h"
#include "utils.h"

#include <set>
#include <sstream>

using namespace Graphics;
using namespace Graphics::OGL;

// ====================================================================
// Shader Setup and Creation
//

Shader::Shader() :
	m_constantStorageSize(0)
{}

Shader::Shader(const std::string &name) :
	m_constantStorageSize(0)
{
	std::string fileName = name + ".shaderdef";
	FileSystem::FileInfo fileInfo = FileSystem::gameDataFiles.Lookup(FileSystem::JoinPathBelow("shaders/opengl", fileName));
	if (!fileInfo.IsFile())
		fileInfo = FileSystem::gameDataFiles.Lookup(FileSystem::JoinPathBelow("shaders", fileName));

	if (!fileInfo.IsFile()) {
		Log::Error("Cannot find shaderdef file {}\n", fileName);
		throw ShaderException();
	}

	RefCountedPtr<FileSystem::FileData> fileData = fileInfo.Read();
	ShaderParser::ShaderInfo info = ShaderParser::Parser().Parse(fileInfo.GetName(), { fileData->GetData(), fileData->GetSize() });

	if (info.name == "<unknown>") {
		Log::Warning("Shaderdef file {} is missing shader name declaration! Defaulting to {}.\n",
			fileInfo.GetName(), name);

		info.name = name;
	}

	if (info.vertexPath.empty()) {
		info.vertexPath = name + ".vert";
		Log::Warning("Shaderdef file {} is missing vertex shader path! Defaulting to {}.\n",
			fileInfo.GetName(), info.vertexPath);
	}

	if (info.fragmentPath.empty()) {
		info.fragmentPath = name + ".frag";
		Log::Warning("Shaderdef file {} is missing fragment shader path! Defaulting to {}.\n",
			fileInfo.GetName(), info.fragmentPath);
	}

	m_programDef.name = info.name;
	m_programDef.vertexShader = FileSystem::JoinPathBelow("shaders/opengl", info.vertexPath);
	m_programDef.fragmentShader = FileSystem::JoinPathBelow("shaders/opengl", info.fragmentPath);

	for (const auto &info : info.textureBindings) {
		AddTextureBinding(info.bindName, info.type, info.binding);
	}

	for (const auto &info : info.bufferBindings) {
		AddBufferBinding(info.name, info.binding);
	}

	for (const auto &info : info.pushConstantBindings) {
		AddConstantBinding(info.name, info.type, info.binding);
	}

	if (GetBufferBindingInfo("DrawData"_hash).binding == InvalidBinding) {
		Log::Warning("Shaderdef file {} is missing DrawData buffer assignment! Defaulting to {}.\n",
			fileInfo.GetName(), info.bufferBindings.back().binding + 1);
		AddBufferBinding("DrawData", info.bufferBindings.back().binding + 1);
	}
}

Shader::~Shader()
{
	while (!m_variants.empty()) {
		delete m_variants.back().second;
		m_variants.pop_back();
	}
}

Program *Shader::GetProgramForDesc(const MaterialDescriptor &desc)
{
	for (auto &pair : m_variants)
		if (pair.first == desc)
			return pair.second;

	Program *program = LoadProgram(desc);
	if (program->Loaded())
		m_variants.push_back({ desc, program });

	return program;
}

void Shader::Reload()
{
	// TODO: reload the shader definition file and regenerate
	// binding points. This is probably not doable without some
	// way to also track and reload all materials using this shader
	// (as they will need to have their data store reallocated as well)

	// For right now, simply reload the underlying shader program
	// source from disk and recompile; changes to the data interface
	// will require a program restart
	for (auto &variant : m_variants) {
		ProgramDef def = m_programDef;
		def.defines = GetProgramDefines(variant.first);
		variant.second->Reload(this, def);
	}
}

Program *Shader::LoadProgram(const MaterialDescriptor &desc)
{
	ProgramDef def = m_programDef;
	def.defines = GetProgramDefines(desc);

	return new Program(this, def);
}

std::string Shader::GetProgramDefines(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;

	ss << "#define PI 3.141592653589793\n";
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	if (desc.lighting && desc.dirLights > 0)
		ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", 1.0f / float(desc.dirLights));

	if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA)
		ss << "#define TERRAIN_WITH_LAVA\n";
	if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER)
		ss << "#define TERRAIN_WITH_WATER\n";
	if (desc.effect == EFFECT_BILLBOARD_ATLAS)
		ss << "#define USE_SPRITE_ATLAS\n";

	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";
	if (desc.vertexColors)
		ss << "#define VERTEXCOLOR\n";
	if (desc.alphaTest)
		ss << "#define ALPHA_TEST\n";

	if (desc.normalMap && desc.lighting && desc.dirLights > 0)
		ss << "#define MAP_NORMAL\n";
	if (desc.specularMap)
		ss << "#define MAP_SPECULAR\n";
	if (desc.glowMap)
		ss << "#define MAP_EMISSIVE\n";
	if (desc.ambientMap)
		ss << "#define MAP_AMBIENT\n";
	if (desc.usePatterns)
		ss << "#define MAP_COLOR\n";

	if (desc.quality & HAS_ATMOSPHERE)
		ss << "#define ATMOSPHERE\n";
	if (desc.quality & HAS_HEAT_GRADIENT)
		ss << "#define HEAT_COLOURING\n";
	if (desc.quality & HAS_ECLIPSES)
		ss << "#define ECLIPSE\n";
	// HACK: pass the number of FBM noise octaves from the high sixteen bits as a compile-time constant to the shader
	// This (and the entire function) is a hack in need of a more generic decoupled system to generate shader variants.
	// 95% of these defines are only used by one or two specific shaders, and even then rarely.
	if (desc.quality & HAS_OCTAVES)
		ss << stringf("#define FBM_OCTAVES %0{u}\n", (desc.quality >> 16) & 0xFFFF);

	if (desc.instanced)
		ss << "#define USE_INSTANCING\n";

	return ss.str();
}

// ====================================================================
// Shader Reflection Information
//

uint32_t CalcSize(ConstantDataFormat format)
{
	using CD = ConstantDataFormat;
	switch (format) {
	case CD::DATA_FORMAT_INT:
	case CD::DATA_FORMAT_FLOAT:
		return 4;
	case CD::DATA_FORMAT_FLOAT3:
		return 4 * 3;
	case CD::DATA_FORMAT_FLOAT4:
		return 4 * 4;
	case CD::DATA_FORMAT_MAT3:
		return 4 * 12;
	case CD::DATA_FORMAT_MAT4:
		return 4 * 16;
	default:
		assert(false);
		return 0;
	}
}

uint32_t CalcOffset(uint32_t last, ConstantDataFormat lastFormat, ConstantDataFormat thisFormat)
{
	using CD = ConstantDataFormat;
	if (thisFormat == CD::DATA_FORMAT_INT || thisFormat == CD::DATA_FORMAT_FLOAT)
		// float and integer types align to 1n, where n is the size of the underlying type.
		// Because we're only dealing with increments of 1n, they're already aligned
		return last + CalcSize(lastFormat);
	else {
		// OpenGL requires uniform buffers to align vec and mat elements to increments of 4n,
		// where n is the size of the underlying type.
		// To preserve forwards compatibility, follow these alignment rules.
		constexpr uint32_t ALIGNMENT = 16 - 1;
		uint32_t offset = last + CalcSize(lastFormat);
		offset = (offset + ALIGNMENT) & ~ALIGNMENT;
		return offset;
	}
}

size_t Shader::AddBufferBinding(const std::string &name, uint32_t binding)
{
	size_t hash = Renderer::GetName(name);
	m_bufferBindingInfo.push_back({ hash, uint32_t(m_bufferBindingInfo.size()), binding, 0 });
	m_nameMap[hash] = name;
	return hash;
}

size_t Shader::AddTextureBinding(const std::string &name, TextureType type, uint32_t binding)
{
	size_t hash = Renderer::GetName(name);
	m_textureBindingInfo.push_back({ hash, uint32_t(m_textureBindingInfo.size()), binding, type });
	m_nameMap[hash] = name;
	return hash;
}

size_t Shader::AddConstantBinding(const std::string &name, ConstantDataFormat format, uint32_t binding)
{
	size_t hash = Renderer::GetName(name);
	GLuint offset = 0;
	if (m_pushConstantInfo.size()) {
		auto &lastConstant = m_pushConstantInfo.back();
		offset = CalcOffset(lastConstant.offset, lastConstant.format, format);
	}

	m_pushConstantInfo.push_back({ hash, uint32_t(m_pushConstantInfo.size()), binding, offset, format });
	m_constantStorageSize = offset + CalcSize(format);
	m_nameMap[hash] = name;
	return hash;
}

// simple linear search significantly outperforms std::map for n <= 32
// Luckily, we don't expect more than 32 elems in any of these maps
template <typename T>
const T *vector_search(const std::vector<T> &vec, size_t name)
{
	for (const T &data : vec)
		if (data.name == name)
			return &data;

	return nullptr;
}

TextureBindingData Shader::GetTextureBindingInfo(size_t name) const
{
	auto *data = vector_search(m_textureBindingInfo, name);
	if (data != nullptr) return *data;

	return { 0, 0, InvalidBinding, TextureType::TEXTURE_2D };
}

PushConstantData Shader::GetPushConstantInfo(size_t name) const
{
	auto *data = vector_search(m_pushConstantInfo, name);
	if (data != nullptr) return *data;

	return { 0, 0, InvalidBinding, 0, ConstantDataFormat::DATA_FORMAT_NONE };
}

BufferBindingData Shader::GetBufferBindingInfo(size_t name) const
{
	auto *data = vector_search(m_bufferBindingInfo, name);
	if (data != nullptr) return *data;

	return { 0, 0, InvalidBinding, 0 };
}
