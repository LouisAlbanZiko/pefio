project "c_file"
	location "."
	kind "StaticLib"
	language "C"
	
	files { "**.c", "**.h" }
	includedirs { "../_vendor", "../", "." }

	links { "c_log", "c_core" }

	filter "configurations:Debug"
		defines { "CC_DEBUG" }

	filter "system:linux"
		links { "m" }
		buildoptions "-g"
