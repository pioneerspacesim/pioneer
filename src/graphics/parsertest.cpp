// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSystem.h"
#include "core/Application.h"
#include "core/Log.h"
#include "graphics/ShaderParser.h"

#include <string_view>

class ShaderTest : public Application::Lifecycle {
public:
	void Update(float deltaTime)
	{
		Graphics::ShaderParser::Parser parser;
		auto file = FileSystem::gameDataFiles.ReadFile("shaders/opengl/multi.shaderdef");

		auto data = parser.Parse(file->GetInfo().GetName(), std::string_view(file->GetData(), file->GetSize()));

		Log::Info("Shader {}\n", data.name);
		for (const auto &info : data.textureBindings)
			Log::Info("Texture {} (name='{}') type={}\n", info.bindName, info.name, info.type);
		for (const auto &info : data.bufferBindings)
			Log::Info("Buffer {} binding={}\n", info.name, info.binding);
		for (const auto &info : data.pushConstantBindings)
			Log::Info("PushConstant {} type={} binding={}\n", info.name, info.type, info.binding);

		Log::Info("Vertex {}\n", data.vertexPath);
		Log::Info("Fragment {}\n", data.fragmentPath);

		RequestEndLifecycle();
	}
};

int main(int argc, const char **argv)
{
	Application app;
	app.QueueLifecycle(std::make_shared<ShaderTest>());
	app.Run();

	return 0;
}
