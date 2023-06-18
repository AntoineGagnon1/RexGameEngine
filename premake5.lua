workspace "RexGameEngine"
    configurations { "Debug", "Release" }
    platforms { "Win64" }

    filter "platforms:Win64"
        systemversion "latest"
        defines { "RE_WIN64", "RE_WINDOWS" }
    
    filter "configurations:Debug"
        defines { "RE_DEBUG" }
        symbols "On"
    
    filter "configurations:Release"
        defines { "RE_RELEASE" }
        optimize "On"


local TargetDir = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.platform}"
local ObjDir = "%{wks.location}/obj/%{cfg.buildcfg}-%{cfg.platform}"

externalproject "CSharpApi"
   location "%{wks.location}/CSharpApi"
   uuid "1447E25E-34A0-4BDB-9AA7-FF1CE309BDBD"
   kind "SharedLib"
   language "C#"

project "RexEngine"
	defines {"GLM_FORCE_LEFT_HANDED"}
    location "RexEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
	warnings "Extra"
	flags { "FatalCompileWarnings" }
   
    targetdir (TargetDir .. "/%{prj.name}")
    objdir (ObjDir .. "/%{prj.name}")

    pchheader "REPch.h"
    pchsource "%{prj.name}/src/REPch.cpp"

    dependson { "CSharpApi" }
	
    files { 
        "%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.cpp",
        
        "%{prj.name}/vendor/**.h", 
        "%{prj.name}/vendor/**.hpp",
        "%{prj.name}/vendor/**.c",
		"%{prj.name}/vendor/**.cpp"
	}
    
    includedirs { 
        "%{prj.name}/src",
        "%{prj.name}/vendor"
    }
    
	libdirs { "%{prj.name}/vendor/glfw/lib", "%{prj.name}/vendor/mono/lib/%{cfg.buildcfg}" }
	links { "glfw3", "opengl32.lib", "mono-2.0-sgen.lib" }
	
    -- Disable Pch for vendors
    filter "files:**/vendor/**.**"
    flags {"NoPCH"}
    filter {}
	

project "RexEditor"
	defines {"GLM_FORCE_LEFT_HANDED"}
    location "RexEditor"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
	warnings "Extra"
	flags { "FatalCompileWarnings" }

	ignoredefaultlibraries { "MSVCRT" }
   
    targetdir (TargetDir .. "/%{prj.name}")
    objdir (ObjDir .. "/%{prj.name}")

	pchheader "REDPch.h"
    pchsource "%{prj.name}/src/REDPch.cpp"

    files { 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.cpp",
		
		"%{prj.name}/vendor/**.h", 
        "%{prj.name}/vendor/**.hpp",
        "%{prj.name}/vendor/**.c",
		"%{prj.name}/vendor/**.cpp"
	}

    includedirs { 
	    "RexEngine/",
        "RexEngine/vendor",
		
		"%{prj.name}/src",
        "%{prj.name}/vendor"
    }
	
	links { "RexEngine" }
	
	-- Disable Pch for vendors
    filter "files:**/vendor/**.**"
		flags {"NoPCH"}
	
	filter "configurations:Debug"
		debugdir "%{cfg.targetdir}"
    
	filter {}
	
	prebuildcommands {
		"{COPY} $(SolutionDir)RexEditor/assets/ $(OutDir)/assets", -- assets
		"{COPY} $(SolutionDir)RexEngine/vendor/mono/bin/%{cfg.buildcfg}/ $(OutDir)", -- mono
		"{COPY} $(SolutionDir)RexEngine/vendor/mono/lib/mono/ $(OutDir)/mono/lib",
		"{COPY} $(OutDir)/../CSharpApi/ $(OutDir)/mono/" -- CSharpApi
	}
	
	-- Always run the post build commands
	if os.istarget "Windows" then
	require "vstudio"
	local p = premake;
	local vc = p.vstudio.vc2010;
	
	function disableFastUpToDateCheck(prj, cfg)
		vc.element("DisableFastUpToDateCheck", nil, "true")
	end
	
	p.override(vc.elements, "globalsCondition",
			function(oldfn, prj, cfg)
				local elements = oldfn(prj, cfg)
				elements = table.join(elements, {disableFastUpToDateCheck})
				return elements
			end)
	end

group ""