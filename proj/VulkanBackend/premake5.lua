VulkanBackendInclude = path.getabsolute("../../include", os.getcwd())

project "VulkanBackend"
	kind "StaticLib"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"
	location ""
	targetdir "../../build/%{cfg.buildcfg}"
	objdir "obj/%{cfg.buildcfg}"
	files { "../../src/**.hpp", "../../src/**.cpp", "../../include/**.hpp" }
	
	includedirs {
		"$(VULKAN_SDK)/include",
		YamlInclude,
		SoftwareCoreInclude,
		"../../ext/MagicEnum",
		VulkanBackendInclude,
		VulkanMemoryHandlerInclude,
		VulkanMemoryAllocatorInclude
	}
	
	links {
		"$(VULKAN_SDK)/lib/vulkan-1.lib",
		"$(VULKAN_SDK)/lib/shaderc_shared.lib",
		"SoftwareCore",
		"yaml-cpp",
		"VulkanMemoryHandler"
	}

	filter "system:windows"
		systemversion "latest"
	filter{}
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		runtime "Release"
		optimize "On"

	filter {}