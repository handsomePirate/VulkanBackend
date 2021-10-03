workspace "VulkanBackend"
	architecture "x64"
	configurations { "Debug", "Release" }
	location ""
	
	flags
	{
		"MultiProcessorCompile"
	}

group "ext"
	include "../ext/SoftwareCore/proj/SoftwareCore"
	include "../ext/EverViewport/proj/EverViewport"
	include "../ext/yaml"
group ""
	
include "../proj/VulkanBackend"
include "../proj/Test"