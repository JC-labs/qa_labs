workspace "qa_labs"
    configurations { "debug", "release" }
    platforms { "x64", "x86" }

    newoption {
        trigger = "output_directory",
        description = "A directory path for output binaries to be moved to.",
        value = "path"
    }
    newoption {
        trigger = "build_directory",
        description = "A directory path for temporary files to be generated in.",
        value = "path"
    }

    targetdir (_OPTIONS["output_directory"] or "output/%{cfg.system}_%{cfg.platform}_%{cfg.buildcfg}")
    location (_OPTIONS["build_directory"] or "build")

    cppdialect "C++latest"
    flags "FatalWarnings"
    warnings "Extra"
        
    vpaths {
        ["include"] = "**.h",
        ["include"] = "**.hpp",
        ["source"] = "**.c",
        ["source"] = "**.cpp"
    }
    
    -- temporary fix
    filter "action:xcode*"
        xcodebuildsettings {           
            ["CLANG_CXX_LANGUAGE_STANDARD"] = "c++2a";
        }
        targetdir(_OPTIONS["output_directory"] or "output/%{cfg.system}_%{cfg.buildcfg}")
    filter "action:gmake*"
        buildoptions "-std=c++2a"
    
    filter "configurations:debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Debug"

    filter "configurations:release"
        defines { "NDEBUG" }
        optimize "Full"

    filter "platforms:x86" architecture "x86"

    filter "platforms:x64" architecture "x86_64"
        
    filter { "system:windows", "action:vs*"}
        flags { "MultiProcessorCompile", "NoMinimalRebuild" }
        linkoptions { "/ignore:4099" }

    filter {}

project "exception_manager"
    kind "StaticLib"
    language "C++"
    includedirs "include/exception_manager"
    location (_OPTIONS["build_directory"] or "build")

    files { 
        "include/exception_manager/**.hpp", 
        "source/exception_manager/**.cpp"
    }

project "exception_manager_test"
    kind "ConsoleApp"
    language "C++"
    includedirs "include/exception_manager"
    location (_OPTIONS["build_directory"] or "build")

    require "script/include_doctest"
    include_doctest()

    links "exception_manager"

    files {
        "test/exception_manager/**.hpp",
        "test/exception_manager/**.cpp",
        "test/main.cpp"
    }

