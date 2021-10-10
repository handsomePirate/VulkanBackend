project "Test"
	kind "ConsoleApp"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"
	location ""
	targetdir "../../Test/build/%{cfg.buildcfg}"
	objdir "obj/%{cfg.buildcfg}"
	files { "../../Test/src/**.hpp", "../../Test/src/**.cpp" }
	
	includedirs {
		"$(VULKAN_SDK)/include",
		SoftwareCoreInclude,
		YamlInclude,
		VulkanBackendInclude
	}
	
	links {
		"$(VULKAN_SDK)/lib/vulkan-1.lib",
		"SoftwareCore",
		"yaml-cpp",
		"VulkanBackend"
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