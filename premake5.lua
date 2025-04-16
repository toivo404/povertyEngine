	-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["spdlog"] = "PovertyEngine/vendor/spdlog/include"
IncludeDir["GLAD"] = "PovertyEngine/vendor/Glad"
IncludeDir["GLM"] = "vendor/glm"
-- Engine Project
project "PovertyEngine"
	location "povertyEngine"
	kind "StaticLib" -- Creates a DLL
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.GLAD}/glad.c",
--		"vendor/flecs/**.h" ,
--			"vendor/flecs/**.c", 
		"vendor/imgui/**.cpp",
		"vendor/imgui/**.h",
		"vendor/stb/stb_image.h",
		"vendor/lohmann/json.hpp"

		--"vendor/imgui/backends/**.c",
		--"vendor/imgui/backends/**.h"
	}
	
	libdirs
	{
		"PovertyEngine/vendor/SDL2/lib/",
		"vendor/Assimp/lib"
	}

	includedirs
	{
		"PovertyEngine/vendor/SDL2/include",
		"%{prj.name}/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLAD}/",
		"%{IncludeDir.GLM}/",
		"vendor/Assimp/include/",
		"vendor/lohmann/",
---			"vendor/flecs/",
		"vendor/imgui/",
		"vendor/stb/",
	--	"vendor/imgui/backends"
	}

	links
	{
		"SDL2",
		"SDL2main",
		"assimp-vc143-mt"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PE_PLATFORM_WINDOWS",
			--"PE_BUILD_DLL",
			"PE_STATIC",
			"FLECS_STATIC"
		}



	filter "configurations:Debug"
		defines "PE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "on"

print(">>> povertyEngine premake loaded")
