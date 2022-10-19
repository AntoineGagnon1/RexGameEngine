workspace "RexGameEngine"
    configurations { "Debug", "Release" }
    platforms { "Win64" }

    filter "platforms:Win64"
        systemversion "latest"
        defines { "RE_WIN64" }
    
    filter "configurations:Debug"
        defines { "RE_DEBUG" }
        symbols "On"
    
    filter "configurations:Release"
        defines { "RE_RELEASE" }
        optimize "On"


project "RexEngine"
    location "RexEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
   
    targetdir "bin/%{cfg.buildcfg}-%{cfg.platform}"
    objdir "obj/%{cfg.buildcfg}-%{cfg.platform}/%{prj.name}"

    pchheader "REPch.h"
    pchsource "%{prj.name}/src/REPch.cpp"

    files { 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/**.h", 
		"%{prj.name}/vendor/**.cpp" 
	}

    includedirs { 
        "%{prj.name}/src",
        "%{prj.name}/vendor"
    }
    

project "RexEditor"
    location "RexEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    links { "RexEngine" }
   
    targetdir "bin/%{cfg.buildcfg}-%{cfg.platform}/%{prj.name}"
    objdir "obj/%{cfg.buildcfg}-%{cfg.platform}/%{prj.name}"

    files { 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.cpp"
	}

    includedirs { 
        "RexEngine",
        "RexEngine/vendor"
    }