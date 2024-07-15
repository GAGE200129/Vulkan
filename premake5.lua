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

   
   pchheader "pch.hpp"

   files {
      "%{prj.location}/src/**.hpp",
      "%{prj.location}/src/**.cpp",
      "%{prj.location}/src/**.c",
      "%{prj.location}/ThirdParty/**.hpp",
      "%{prj.location}/ThirdParty/**.cpp",
      "%{prj.location}/ThirdParty/**.c",
      "%{prj.location}/shaders/**.vert",
      "%{prj.location}/shaders/**.geom",
      "%{prj.location}/shaders/**.frag",
      "%{prj.location}/shaders/**.comp",
   }
   
   
   links { "glfw", "vulkan", "luajit-5.1", "bfd", "GL"}
   includedirs { 
      "%{prj.location}/src",
      "%{prj.location}/ThirdParty",
      "%{wks.location}"
   }

   
   filter { "files:**.c" }
      compileas "C++"

   --Shader compiling--
   filter {"files:**.vert"}
      buildmessage "Compiling vertex shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{file.directory}/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{file.directory}/compiled/%{file.name}.spv" }

   filter {"files:**.geom"}
      buildmessage "Compiling geometry shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{file.directory}/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{file.directory}/compiled/%{file.name}.spv" }

   filter {"files:**.frag"}
      buildmessage "Compiling fragment shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{file.directory}/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{file.directory}/compiled/%{file.name}.spv" }

   filter {"files:**.comp"}
      buildmessage "Compiling compute shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{file.directory}/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{file.directory}/compiled/%{file.name}.spv" }


   
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


   