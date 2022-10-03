workspace "KTL"
    architecture "x64"
    startproject "Test"
    
    configurations {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("bin/%{outputdir}")
    objdir ("bin-obj/%{outputdir}")

    files {
        "src/**.cpp",
        "src/**.h",
        "include/**.h"
    }

    includedirs {
        "src",
        "include"
    }

    filter "system:windows"
        systemversion "latest"
        
        flags { "MultiProcessorCompile" }

    filter "system:linux"
        systemversion "latest"
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        
    filter "configurations:Release"
        flags { "LinkTimeOptimization" }

        runtime "Release"
        optimize "on"