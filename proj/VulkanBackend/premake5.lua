project "VulkanBackend"
	kind "StaticLib"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"
	location ""
	targetdir "../../build/%{cfg.buildcfg}"
	objdir "obj/%{cfg.buildcfg}"
	files { "../../src/**.hpp", "../../src/**.cpp" }
	
	includedirs {
		"../../ext/SoftwareCore/src",
		"../../ext/yaml/include",
		"../../ext/MagicEnum",
		"../../src",
		"$(VULKAN_SDK)/include"
	}
	
	links {
		"$(VULKAN_SDK)/lib/vulkan-1.lib",
		"$(VULKAN_SDK)/lib/shaderc_shared.lib",
		"SoftwareCore",
		"yaml-cpp"
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