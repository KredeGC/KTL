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

newoption {
    trigger = "dialect",
    value = "Dialect (eg. C++17, C++20)",
    description = "The dialect to use when generating project files",
    default = "C++17",
}

require "scripts/build"
require "scripts/test"

workspace "KTL"
    architecture "x64"
    startproject "Test"
    toolset(_OPTIONS["toolset"])
    
    configurations {
        "debug",
        "release"
    }
    
    filter "toolset:clang"
        linkoptions { "-fuse-ld=lld" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Test"
    kind "ConsoleApp"
    language "C++"
    cppdialect(_OPTIONS["dialect"])
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
        
        conformancemode "on"
        
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