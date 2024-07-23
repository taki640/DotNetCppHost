-- premake5.lua
workspace "DotNetCppHost"
    architecture "x64"
    startproject "DotNetCppHost"
    configurations {
        "Debug",
        "Release"
    }

    -- Workspace-wide build options for MSVC
    filter "system:windows"
    buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

externalproject "DotNetLib"
    location "%{prj.name}"
    language "C#"
    kind "SharedItems"

project "DotNetCppHost"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    location "%{prj.name}"
    targetdir ("%{wks.location}/Bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Bin/Int/" .. OutputDir .. "/%{prj.name}")

    files {
        "Premake5.lua",
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.hpp",
        "%{prj.name}/Source/**.cpp"
    }

    links {
        "nethost"
    }

    includedirs {
        "%{prj.name}/Source",
        -- Requires .NET runtime to be installed
        "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/8.0.7/runtimes/win-x64/native"
    }

    libdirs {
        "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/8.0.7/runtimes/win-x64/native"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"
