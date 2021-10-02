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
		"../../ext/SoftwareCore/src",
		"../../ext/yaml/include",
		"../../src",
		"../../Test/src"
	}
	
	links {
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