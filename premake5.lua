-- premake5.lua
workspace "GAGE"
   configurations { "Debug", "Release" }
   toolset "gcc"
   language "C++"
   cppdialect "c++20"

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

project "Core"
   location "Core"
   kind "SharedLib"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "obj/%{prj.name}/%{cfg.buildcfg}"

   files {
      "%{prj.location}/src/**.hpp",
      "%{prj.location}/src/**.cpp",
      "%{prj.location}/ThirdParty/**.hpp",
      "%{prj.location}/ThirdParty/**.cpp",
   }
   links { "glfw", "vulkan", "fmt", "luajit-5.1", "bfd"}
   includedirs { 
      "%{prj.location}/src",
      "%{prj.location}/ThirdParty",
      "%{wks.location}"
   }
   
   --Linux--
   filter { "system:linux" }
   buildoptions 
   {
      "-Wall -Wextra -Wpedantic"
   }


project "Sandbox"
   location "Sandbox"
   kind "ConsoleApp"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "obj/%{prj.name}/%{cfg.buildcfg}"

   files {
      "%{prj.location}/**.hpp", "%{prj.location}/**.cpp",
   }
   dependson { "Core" }
   links { "Core" }
   includedirs { 
      "%{prj.location}",
      "%{wks.location}"
   }


   