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
		VulkanMemoryAllocatorInclude
	}
	
	links {
		"SoftwareCore",
		"yaml-cpp",
		"VulkanMemoryAllocator"
	}

	filter "system:windows"
		links {
			"$(VULKAN_SDK)/lib/vulkan-1.lib"
		}
	filter "system:linux"
		links {
			"vulkan"
		}
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