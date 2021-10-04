#include <yaml-cpp/yaml.h>
#include <VulkanBackendAPI.hpp>
#include <Filesystem/Filesystem.hpp>
#include <iostream>

void ConsoleOutput(const char* message, ::Core::LoggerSeverity severity)
{
	switch (severity)
	{
	case Core::LoggerSeverity::Fatal:
		std::cout << "[Fatal] " << message;
		break;
	case Core::LoggerSeverity::Error:
		std::cout << "[Error] " << message;
		break;
	case Core::LoggerSeverity::Warn:
		std::cout << "[Warn] " << message;
		break;
	case Core::LoggerSeverity::Info:
		std::cout << "[Info] " << message;
		break;
	case Core::LoggerSeverity::Debug:
		std::cout << "[Debug] " << message;
		break;
	case Core::LoggerSeverity::Trace:
		std::cout << "[Trace] " << message;
		break;
	}
}

int main(int argc, char* argv[])
{
	VulkanLogger.SetNewOutput(ConsoleOutput);

	Core::Filesystem filesystem(argv[0]);
	std::string pathToYamlFile = filesystem.GetAbsolutePath("../../testfile.yml");

	VulkanBackend::Initialized initialized = VulkanBackend::Initialize(pathToYamlFile.c_str());

	VulkanBackend::Shutdown(initialized);

	return 0;
}