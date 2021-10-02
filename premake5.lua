workspace "VulkanBackend"
	architecture "x64"
	configurations { "Debug", "Release" }
	location "proj"
	
	flags
	{
		"MultiProcessorCompile"
	}

project "VulkanBackend"
	kind "StaticLib"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"
	location "proj/VulkanBackend"
	targetdir "./build/%{cfg.buildcfg}"
	objdir "proj/VulkanBackend/obj/%{cfg.buildcfg}"
	files { "src/**.hpp", "src/**.cpp" }
	
	includedirs {
		"src",
		"$(VULKAN_SDK)/include"
	}
	
	links {
		"$(VULKAN_SDK)/lib/vulkan-1.lib",
		"$(VULKAN_SDK)/lib/shaderc_shared.lib"
	}

	filter "system:windows"
		systemversion "latest"
	filter{}
	
	filter "configurations:Debug"
		defines { "VB_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "VB_RELEASE" }
		runtime "Release"
		optimize "On"

	filter {}
	
project "Test"
	kind "ConsoleApp"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"
	location "proj/Test"
	targetdir "./Test/build/%{cfg.buildcfg}"
	objdir "proj/Test/obj/%{cfg.buildcfg}"
	files { "Test/src/**.hpp", "Test/src/**.cpp" }
	
	includedirs {
		"src",
		"Test/src"
	}
	
	links {
		"SoftwareCore"
	}
	
	filter "system:windows"
		systemversion "latest"
	filter{}
	
	filter "configurations:Debug"
		defines { "VB_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "VB_RELEASE" }
		runtime "Release"
		optimize "On"

	filter {}
