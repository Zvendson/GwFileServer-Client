workspace "GuildWars"
    system ("windows")
    architecture ("x86")
    characterset ("MBCS")
    startproject ("Nika")

    configurations
    {
        "Debug",
        "Release",
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}"
    target_dir = "%{wks.location.name}"

    project "FileServer-Client"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        staticruntime "on"

        targetdir ("%{wks.location}/bin")
        symbolspath ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
        targetname ("%{prj.name}_%{cfg.buildcfg}")

        files
        {
                "FileServerClient/**.h",
                "FileServerClient/**.cpp"
        }

        includedirs
        {
                "FileServerClient",
        }

        defines 
        { 
                "NOMINMAX", 
                "WIN32_LEAN_AND_MEAN", 
        }

        filter "configurations:Debug"                
                ignoredefaultlibraries { "libcmt.lib" }
                runtime "Debug"
                symbols "on"

        filter "configurations:Release"
                targetname ("%{prj.name}")
                runtime "Release"
                optimize "on"
                
        filter ""