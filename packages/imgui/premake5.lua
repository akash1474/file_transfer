project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"**.cpp",
		"**.h"
	}

	links{"glfw3"}
	
	includedirs{".","../glfw/include"}
	libdirs{"../glfw/lib"}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		staticruntime "On"
      	buildoptions { "/MP" }
		defines{"SFML_STATIC"}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		staticruntime "On"
      	buildoptions { "/MP" }
		defines{"SFML_STATIC"}

    filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"
		staticruntime "On"
        buildoptions { "/MP" }
        defines{"SFML_STATIC"}
