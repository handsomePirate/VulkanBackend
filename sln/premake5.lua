workspace "VulkanBackend"
	architecture "x64"
	configurations { "Debug", "Release" }
	location ""
	
	flags
	{
		"MultiProcessorCompile"
	}
	
include "../dependencies.lua"
	
include "../proj/VulkanBackend"
include "../proj/Test"