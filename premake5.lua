newoption {
    trigger = "toolset",
    value = "Toolset (eg. gcc, clang, msc)",
    description = "The toolset to use to compile with",
    default = "gcc",
    allowed = {
        { "gcc", "gcc and g++ using ld and ar" },
        { "clang", "clang and clang++ using lld and llvm-ar" },
        { "msc", "msbuild and cl using link.exe" }
    }
}

newoption {
    trigger = "config",
    value = "Configuration",
    description = "The configuration to compile",
    default = "debug",
    allowed = {
        { "debug", "Debug build with symbols turned on" },
        { "release", "Release build without symbols, but with optimizations" }
    }
}

require "scripts/build"

workspace "KTL"
    architecture "x64"
    startproject "Test"
    toolset(_OPTIONS["toolset"])
    
    configurations {
        "Debug",
        "Release"
    }
    
    filter "toolset:clang"
        linkoptions { "-fuse-ld=lld" }

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