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
   
   
   links { "glfw", "vulkan", "bfd", "GL", "Jolt", "spdlog"}
   includedirs { 
      "%{prj.location}/src",
      "%{prj.location}/ThirdParty",
      "%{wks.location}"
   }

   defines 
   {
      "GLFW_INCLUDE_VULKAN",
      "GLM_FORCE_DEPTH_ZERO_TO_ONE",
      "SPDLOG_COMPILED_LIB"
   }

   
   filter { "files:**.c" }
      compileas "C++"

   --Shader compiling--
   filter {"files:**.vert"}
      buildmessage "Compiling vertex shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{prj.location}/shaders/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{prj.location}/shaders/compiled/%{file.name}.spv" }

   filter {"files:**.geom"}
      buildmessage "Compiling geometry shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{prj.location}/shaders/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{prj.location}/shaders/compiled/%{file.name}.spv" }

   filter {"files:**.frag"}
      buildmessage "Compiling fragment shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{prj.location}/shaders/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{prj.location}/shaders/compiled/%{file.name}.spv" }

   filter {"files:**.comp"}
      buildmessage "Compiling compute shader %{file.relpath}"
      buildcommands {
         "glslc %{file.relpath} -o %{prj.location}/shaders/compiled/%{file.name}.spv"
      }
      buildoutputs { "%{prj.location}/shaders/compiled/%{file.name}.spv" }


   
   --Linux--
   filter {"configurations:Debug", "system:linux"}
      buildoptions 
      {
         "-Wall -Wextra -Wpedantic -fsanitize=address -static-libasan"
      }

      linkoptions { "-fsanitize=address -static-libasan" }

      defines 
      {
         "BACKWARD_SYSTEM_LINUX"
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

   filter "Debug"
      buildoptions 
      {
         "-Wall -Wextra -Wpedantic -fsanitize=address -static-libasan"
      }

      linkoptions { "-fsanitize=address -static-libasan" }



   