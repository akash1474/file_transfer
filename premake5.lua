-- Building
-- MsBuild XPlayer.sln /p:configuration=Release

workspace "FileTransfer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

includeDirs={}
includeDirs["ImGui"]="packages/imgui"
includeDirs["LunaSVG"]="packages/lunasvg/include"
includeDirs["SpdLog"]="packages/spdlog/include"
includeDirs["libcpr"]="packages/cpr/include"
includeDirs["glfw"]="packages/glfw/include"
includeDirs["clipboardxx"]="packages/clipboardxx"
includeDirs["mini"]="packages/mini"
include "packages/imgui"
include "packages/lunasvg"


project "file_transfer"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin"
   objdir "bin/obj"
   pchheader "pch.h"
   pchsource "src/pch.cpp"

   links {
      "ImGui","LunaSVG","opengl32","glfw3","cpr","libcurl","zlib","user32","shell32","gdi32",
      -- Curl Additional
      "Normaliz","Ws2_32","Wldap32","Crypt32","advapi32"
   }

   files { 
         "src/**.cpp"
   }

   includedirs{
      "src",
      "%{includeDirs.ImGui}",
      "%{includeDirs.LunaSVG}",
      "%{includeDirs.SpdLog}",
      "%{includeDirs.libcpr}",
      "%{includeDirs.clipboardxx}",
      "%{includeDirs.mini}",
      "%{includeDirs.glfw}"
   }
   libdirs{"packages/cpr/lib","packages/glfw/lib"}

   filter "system:windows"
      systemversion "latest"

   filter "configurations:Debug"
      runtime "Debug"
      symbols "On"
      staticruntime "On"
      optimize "Off"
      characterset ("MBCS")
      buildoptions { "/MP","/utf-8" }

   filter {"configurations:Release"}
      runtime "Release"
      optimize "On"
      symbols "Off"
      characterset ("MBCS")
      staticruntime "On"
      buildoptions { "/MP","/utf-8" }

   filter "configurations:Dist"
      kind "WindowedApp"
      runtime "Release"
      optimize "On"
      symbols "Off"
      characterset ("MBCS")
      staticruntime "On"
