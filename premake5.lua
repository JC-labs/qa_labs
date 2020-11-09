workspace "qa_labs"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }

    targetdir "output/%{cfg.buildcfg}_%{cfg.platform}"
    location "project"

    cppdialect "C++latest"
    flags "FatalWarnings"
    warnings "Extra"
        
    vpaths {
        ["include"] = "**.h",
        ["include"] = "**.hpp",
        ["source"] = "**.c",
        ["source"] = "**.cpp"
    }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Full"

    filter "platforms:x86" architecture "x86"

    filter "platforms:x64" architecture "x64"
        
    filter { "system:windows", "action:vs*"}
        flags { "MultiProcessorCompile", "NoMinimalRebuild" }
        linkoptions { "/ignore:4099" }

    filter {}

project "lab_1"
    kind "StaticLib"
    language "C++"
    location "project/lab_1"
    includedirs "include/lab_1"

    files { 
        "include/lab_1/**.hpp", 
        "source/lab_1/**.cpp"
    }

project "lab_1_test"
    kind "ConsoleApp"
    language "C++"
    location "project/lab_1"
    includedirs "include/lab_1"

    require "script/include_doctest"
    include_doctest()

    links "lab_1"

    files {
        "test/lab_1/**.hpp",
        "test/lab_1/**.cpp",
        "test/main.cpp"
    }

