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
	include "../ext/yaml"
group ""
	
include "../proj/VulkanBackend"
include "../proj/Test"