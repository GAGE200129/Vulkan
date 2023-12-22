-- premake5.lua
workspace "GAGE"
   configurations { "Debug", "Release" }
   

project "EnGAGE"
   kind "ConsoleApp"
   language "C++"
   cppdialect "c++17"
   targetdir "bin/%{cfg.buildcfg}"
   pchheader "src/pch.hpp"

   files {
      "src/**.hpp", "src/**.cpp", "res/**.vert", "res/**.frag",
      "libs/imgui-docking/*.cpp",
      "libs/imgui-docking/backends/imgui_impl_opengl2.cpp",
      "libs/imgui-docking/backends/imgui_impl_glfw.cpp"

   }
   links { "glfw", "vulkan", "spdlog", "fmt", "lua5.4", "assimp", "LinearMath", "BulletCollision", "BulletDynamics", "GL" }
   includedirs { 
      "src",
      "libs/EnTT/single_include",
      "libs/imgui-docking"
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