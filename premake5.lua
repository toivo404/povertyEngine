workspace "PovertyEngine"
    architecture "x64"
    startproject "Game"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- Include directories relative to root folder (solution directory)
    IncludeDir = {}
    IncludeDir["spdlog"] = "PovertyEngine/vendor/spdlog/include"

	ExtDir = {}
	ExtDir["SDL2"] = "PovertyEngine/vendor/SDL2"
	
    -- Engine Project
    project "PovertyEngine"
        location "PovertyEngine"
        kind "SharedLib" -- Creates a DLL
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp"
        }
		
		libdirs
		{
			"%{ExtDir.SDL2}/lib/"
		}

        includedirs
        {
            "%{prj.name}/src",
            "%{IncludeDir.spdlog}",
			"%{ExtDir.SDL2}/include"
        }

        links
        {
			"SDL2",
			"SDL2main"
        }

        filter "system:windows"
            systemversion "latest"

            defines
            {
                "PE_PLATFORM_WINDOWS",
                "PE_BUILD_DLL"
            }

            postbuildcommands
            {
                ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Game")
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

    -- Game Project
    project "Game"
        location "Game"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp"
        }

        includedirs
        {
            "PovertyEngine/src",
            "%{IncludeDir.spdlog}"
        }

        links
        {
            "PovertyEngine"
        }

        filter "system:windows"
            systemversion "latest"

            defines
            {
                "PE_PLATFORM_WINDOWS"
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
