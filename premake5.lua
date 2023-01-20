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

group "Engine"

project "RexEngine"
	defines {"GLM_FORCE_LEFT_HANDED"}
    location "RexEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
	warnings "Extra"
	flags { "FatalCompileWarnings" }
	
	dependson { "ScriptEngine", "ScriptApi" }
   
    targetdir (TargetDir .. "/%{prj.name}")
    objdir (ObjDir .. "/%{prj.name}")

    pchheader "REPch.h"
    pchsource "%{prj.name}/src/REPch.cpp"

    
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
    
	libdirs { "%{prj.name}/vendor/glfw/lib", "%{prj.name}/vendor/dotnet" }
	links { "glfw3", "opengl32.lib", "nethost.lib" }
	
    -- Disable Pch for vendors
    filter "files:**/vendor/**.**"
    flags {"NoPCH"}
    filter {}
	
	
externalproject "ScriptEngine"
   location "%{wks.location}/ScriptEngine"
   uuid "308A4845-1CED-D7E9-C572-10A0B1B4A36C"
   kind "SharedLib"
   language "C#"
   
externalproject "ScriptApi"
   location "%{wks.location}/ScriptApi"
   uuid "C0575995-79D9-48EE-B4DE-4F0FF0FC7F1F"
   kind "SharedLib"
   language "C#"
   
group "Editor"

externalproject "RexEditorScript"
   location "%{wks.location}/RexEditorScript"
   uuid "7BF78010-D122-4748-B3D5-5C39DE22E2AD"
   kind "SharedLib"
   language "C#"
	
	

project "RexEditor"
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

	dependson { "RexEditorScript" }

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
		"{COPY} $(SolutionDir)RexEngine/vendor/dotnet/nethost.dll $(OutDir)", -- copy the nethost dll
		"{COPY} ".. TargetDir .."/ScriptEngine/ScriptEngine.dll $(OutDir)Dotnet/ScriptEngine", -- copy the ScriptEngine files
		"{COPY} ".. TargetDir .."/ScriptEngine/ScriptEngine.runtimeconfig.json $(OutDir)Dotnet/ScriptEngine",
		"{COPY} ".. TargetDir .."/ScriptApi/ScriptApi.dll $(OutDir)Dotnet/ScriptEngine", -- ScriptApi
		"{COPY} ".. TargetDir .."/RexEditorScript/RexEditorScript.dll $(OutDir)Dotnet/Editor", -- RexEditorScript
		"{COPY} $(SolutionDir)RexEditor/assets/ $(OutDir)/assets", -- assets
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