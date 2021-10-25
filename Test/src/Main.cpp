#include <yaml-cpp/yaml.h>
#include <VulkanBackend/VulkanBackendAPI.hpp>
#include <SoftwareCore/Filesystem.hpp>
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

	VulkanBackend::BackendData backendData = VulkanBackend::Initialize(pathToYamlFile.c_str());

	VulkanBackend::CreateBuffer(backendData, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 24, VMA_MEMORY_USAGE_GPU_ONLY);

	VulkanBackend::Shutdown(backendData);

	return 0;
}