-- premake5.lua
workspace "GAGE"
   configurations { "Debug", "Release" }


project "EnGAGEditor"
   kind "ConsoleApp"
   language "C++"
   cppdialect "c++17"
   targetdir "bin/%{cfg.buildcfg}"

   files { "editor/**.hpp", "editor/**.cpp", "res/**.vert", "res/**.frag" }
   links { "glfw", "spdlog", "fmt", "lua5.4", "assimp" }

   includedirs { 
      "src/editor"
   }

   

project "EnGAGE"
   kind "ConsoleApp"
   language "C++"
   cppdialect "c++17"
   targetdir "bin/%{cfg.buildcfg}"
   pchheader "src/pch.hpp"

   files { "src/**.hpp", "src/**.cpp", "res/**.vert", "res/**.frag" }
   links { "glfw", "vulkan", "spdlog", "fmt", "lua5.4", "assimp", "LinearMath", "BulletCollision", "BulletDynamics" }
   includedirs { 
      "src",
      "libs/EnTT/single_include"
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


   --Shader build step--
   filter {'files:res/**.vert'}
      buildmessage 'Compiling %{file.relpath}'

      buildcommands {
         'glslc "%{file.relpath}" -o "%{file.relpath}.spv"'
      }
      buildoutputs { '%{file.relpath}.spv' }


   filter {'files:res/**.frag'}
      buildmessage 'Compiling %{file.relpath}'
   
      buildcommands {
         'glslc "%{file.relpath}" -o "%{file.relpath}.spv"'
      }
      buildoutputs { '%{file.relpath}.spv' }