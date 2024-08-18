require "vendor/premake-ecc/ecc"

workspace "anido"
   location "./build"
   configurations { "debug", "release" }

project "anido"
   kind "ConsoleApp"
   language "C"
   targetdir "bin/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.c" }
   links { "curl", "xml2", "ssl", "crypto" }
   includedirs { "/usr/include/libxml2" }

   filter "configurations:debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:release"
      defines { "NDEBUG" }
      optimize "On"
