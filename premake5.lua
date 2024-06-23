-- premake5.lua
workspace "GAGE"
   configurations { "Debug", "Release" }
   toolset "gcc"
   language "C++"
   cppdialect "c++20"

project "Core"
   location "Core"
   kind "StaticLib"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "obj/%{prj.name}/%{cfg.buildcfg}"

   files {
      "%{prj.location}/src/**.hpp", "%{prj.location}/src/**.cpp",
   }
   links { "glfw", "vulkan", "spdlog", "fmt", "luajit-5.1", "LinearMath", "BulletCollision", "BulletDynamics", "bfd"}
   includedirs { 
      "%{prj.location}/src",
      "%{prj.location}/ThirdParty",
      "%{wks.location}"
   }
   
   defines 
   {
      "SPDLOG_COMPILED_LIB"
   }


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   --Linux--
   filter { "system:linux" }
   buildoptions 
   {
      "-Wall -Wextra -Wpedantic"
   }

project "UnitTest"
   location "UnitTest"
   kind "ConsoleApp"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "obj/%{prj.name}/%{cfg.buildcfg}"

   files {
      "%{prj.location}/**.hpp", "%{prj.location}/**.cpp",
   }
   links { "Core", "bfd" }
   includedirs { 
      "%{prj.location}",
      "%{wks.location}"
   }


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"


project "Sandbox"
   location "Sandbox"
   kind "ConsoleApp"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "obj/%{prj.name}/%{cfg.buildcfg}"

   files {
      "%{prj.location}/**.hpp", "%{prj.location}/**.cpp",
   }
   links { "Core", "bfd" }
   includedirs { 
      "%{prj.location}",
      "%{wks.location}"
   }


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"


   